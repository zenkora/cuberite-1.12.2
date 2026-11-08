#include "Globals.h"
#include "ChunkDataSerializer.h"
#include "../ClientHandle.h"
#include "../WorldStorage/FastNBT.h"
#include "CircularBufferCompressor.h"

namespace
{
	std::pair<UInt16, size_t> GetSectionBitmask(const ChunkBlockData & a_BlockData, const ChunkLightData & a_LightData)
	{
		size_t Present = 0;
		UInt16 Mask = 0;

		ChunkDef_ForEachSection(a_BlockData, a_LightData,
		{
			Present++;
			Mask |= (1 << Y);
		});

		return { Mask, Present };
	}

	auto PaletteLegacy(const BLOCKTYPE a_BlockType, const NIBBLETYPE a_Meta)
	{
	    return (a_BlockType << 4) | a_Meta;
	}

	// Compression threshold for chunk data serialization (same value as Protocol_1_8.cpp: 256)
	static const UInt32 ChunkCompressionThreshold = 256;

	void CompressPacket(CircularBufferCompressor & a_Packet, ContiguousByteBuffer & a_CompressedData)
	{
		const auto Uncompressed = a_Packet.GetView();

		if (Uncompressed.size() < ChunkCompressionThreshold)
		{
			/* Size doesn't reach threshold, not worth compressing.

			--------------- Packet format ----------------
			|--- Header ---------------------------------|
			| PacketSize: Size of all fields below       |
			| DataSize: Zero, means below not compressed |
			|--- Body -----------------------------------|
			| a_Packet: copy of uncompressed data        |
			----------------------------------------------
			*/
			const UInt32 DataSize = 0;
			const auto PacketSize = static_cast<UInt32>(cByteBuffer::GetVarIntSize(DataSize) + Uncompressed.size());

			cByteBuffer LengthHeaderBuffer(
				cByteBuffer::GetVarIntSize(PacketSize) +
				cByteBuffer::GetVarIntSize(DataSize)
			);

			LengthHeaderBuffer.WriteVarInt32(PacketSize);
			LengthHeaderBuffer.WriteVarInt32(DataSize);

			ContiguousByteBuffer LengthData;
			LengthHeaderBuffer.ReadAll(LengthData);

			a_CompressedData.reserve(LengthData.size() + Uncompressed.size());
			a_CompressedData = LengthData;
			a_CompressedData += Uncompressed;

			return;
		}

		/* Definitely worth compressing.

		--------------- Packet format ----------------
		|--- Header ---------------------------------|
		| PacketSize: Size of all fields below       |
		| DataSize: Size of uncompressed a_Packet    |
		|--- Body -----------------------------------|
		| CompressedData: compressed a_Packet        |
		----------------------------------------------
		*/
		const auto CompressedData = a_Packet.Compress();
		const auto Compressed = CompressedData.GetView();
		const UInt32 DataSize = static_cast<UInt32>(Uncompressed.size());
		const auto PacketSize = static_cast<UInt32>(cByteBuffer::GetVarIntSize(DataSize) + Compressed.size());

		cByteBuffer LengthHeaderBuffer(
			cByteBuffer::GetVarIntSize(PacketSize) +
			cByteBuffer::GetVarIntSize(DataSize)
		);

		LengthHeaderBuffer.WriteVarInt32(PacketSize);
		LengthHeaderBuffer.WriteVarInt32(DataSize);

		ContiguousByteBuffer LengthData;
		LengthHeaderBuffer.ReadAll(LengthData);

		a_CompressedData.reserve(LengthData.size() + Compressed.size());
		a_CompressedData = LengthData;
		a_CompressedData += Compressed;
	}
}

cChunkDataSerializer::cChunkDataSerializer(const eDimension a_Dimension) :
	m_Packet(512 KiB),
	m_Dimension(a_Dimension)
{
}

void cChunkDataSerializer::SendToClients(const int a_ChunkX, const int a_ChunkZ, const ChunkBlockData & a_BlockData, const ChunkLightData & a_LightData, const unsigned char * a_BiomeMap, const ClientHandles & a_SendTo)
{
	for (const auto & Client : a_SendTo)
	{
		// 1.12.2 only - use CacheVersion::v110
		Serialize(Client, a_ChunkX, a_ChunkZ, a_BlockData, a_LightData, a_BiomeMap, CacheVersion::v110);
	}

	// Our cache is only persistent during the function call:
	for (auto & Cache : m_Cache)
	{
		Cache.Engaged = false;
	}
}

inline void cChunkDataSerializer::Serialize(const ClientHandles::value_type & a_Client, const int a_ChunkX, const int a_ChunkZ, const ChunkBlockData & a_BlockData, const ChunkLightData & a_LightData, const unsigned char * a_BiomeMap, const CacheVersion a_CacheVersion)
{
	auto & Cache = m_Cache[static_cast<size_t>(a_CacheVersion)];
	if (Cache.Engaged)
	{
		// Success! We've done it already, just re-use:
		a_Client->SendChunkData(a_ChunkX, a_ChunkZ, Cache.ToSend);
		return;
	}

	// Only v110 is supported for 1.12.2
	Serialize110(a_ChunkX, a_ChunkZ, a_BlockData, a_LightData, a_BiomeMap);

	CompressPacketInto(Cache);
	ASSERT(Cache.Engaged);  // Cache must be populated now
	a_Client->SendChunkData(a_ChunkX, a_ChunkZ, Cache.ToSend);
}

inline void cChunkDataSerializer::Serialize110(const int a_ChunkX, const int a_ChunkZ, const ChunkBlockData & a_BlockData, const ChunkLightData & a_LightData, const unsigned char * a_BiomeMap)
{
	// This function returns the fully compressed packet (including packet size), not the raw packet!
	// Below variables tagged static because of https://developercommunity.visualstudio.com/content/problem/367326

	static constexpr UInt8 BitsPerEntry = 13;
	static constexpr size_t ChunkSectionDataArraySize = (ChunkBlockData::SectionBlockCount * BitsPerEntry) / 8 / 8;  // Convert from bit count to long count

	const auto Bitmask = GetSectionBitmask(a_BlockData, a_LightData);

	// Create the packet:
	m_Packet.WriteVarInt32(0x20);  // Packet id (Chunk Data packet)
	m_Packet.WriteBEInt32(a_ChunkX);
	m_Packet.WriteBEInt32(a_ChunkZ);
	m_Packet.WriteBool(true);        // "Ground-up continuous", or rather, "biome data present" flag
	m_Packet.WriteVarInt32(Bitmask.first);

	size_t ChunkSectionSize = (
		1 +                                // Bits per block - set to 13, so the global palette is used and the palette has a length of 0
		1 +                                // Palette length
		2 +                                // Data array length VarInt - 2 bytes for the current value
		ChunkSectionDataArraySize * 8 +    // Actual block data - multiplied by 8 because first number is longs
		ChunkLightData::SectionLightCount  // Block light
	);

	if (m_Dimension == dimOverworld)
	{
		// Sky light is only sent in the overworld.
		ChunkSectionSize += ChunkLightData::SectionLightCount;
	}

	const size_t BiomeDataSize = cChunkDef::Width * cChunkDef::Width;
	const size_t ChunkSize = (
		ChunkSectionSize * Bitmask.second +
		BiomeDataSize
	);

	// Write the chunk size:
	m_Packet.WriteVarInt32(static_cast<UInt32>(ChunkSize));

	// Write each chunk section...
	ChunkDef_ForEachSection(a_BlockData, a_LightData,
	{
		m_Packet.WriteBEUInt8(BitsPerEntry);
		m_Packet.WriteVarInt32(0);  // Palette length is 0
		m_Packet.WriteVarInt32(static_cast<UInt32>(ChunkSectionDataArraySize));
		WriteBlockSectionSeamless<&PaletteLegacy>(Blocks, Metas, BitsPerEntry);
		WriteLightSectionGrouped(BlockLights, SkyLights);
	});

	// Write the biome data
	m_Packet.WriteBuf(a_BiomeMap, BiomeDataSize);

	// Identify 1.9.4's tile entity list as empty
	m_Packet.WriteBEUInt8(0);
}

template <auto Palette>
inline void cChunkDataSerializer::WriteBlockSectionSeamless(const ChunkBlockData::BlockArray * a_Blocks, const ChunkBlockData::MetaArray * a_Metas, const UInt8 a_BitsPerEntry)
{
	// https://wiki.vg/Chunk_Format#Data_structure

	// We shift a UInt64 by a_BitsPerEntry, the latter cannot be too big:
	ASSERT(a_BitsPerEntry < 64);

	UInt64 Buffer = 0;  // A buffer to compose multiple smaller bitsizes into one 64-bit number
	unsigned char BitIndex = 0;  // The bit-position in Buffer that represents where to write next

	const bool BlocksExist = a_Blocks != nullptr;
	const bool MetasExist = a_Metas != nullptr;

	for (size_t Index = 0; Index != ChunkBlockData::SectionBlockCount; Index++)
	{
		const BLOCKTYPE BlockType = BlocksExist ? (*a_Blocks)[Index] : 0;
		const NIBBLETYPE BlockMeta = MetasExist ? cChunkDef::ExpandNibble(a_Metas->data(), Index) : 0;
		const auto Value = Palette(BlockType, BlockMeta);

		// Write as much as possible of Value, starting from BitIndex, into Buffer:
		Buffer |= static_cast<UInt64>(Value) << BitIndex;

		// The _signed_ count of bits in Value left to write
		const auto Remaining = static_cast<char>(a_BitsPerEntry - (64 - BitIndex));
		if (Remaining >= 0)
		{
			// There were some bits remaining: we've filled the buffer. Flush it:
			m_Packet.WriteBEUInt64(Buffer);

			// And write the remaining bits, setting the new BitIndex:
			Buffer = static_cast<UInt64>(Value >> (a_BitsPerEntry - Remaining));
			BitIndex = static_cast<unsigned char>(Remaining);
		}
		else
		{
			// It fit, excellent.
			BitIndex += a_BitsPerEntry;
		}
	}

	static_assert((ChunkBlockData::SectionBlockCount % 64) == 0, "Section must fit wholly into a 64-bit long array");
	ASSERT(BitIndex == 0);
	ASSERT(Buffer == 0);
}

inline void cChunkDataSerializer::WriteLightSectionGrouped(const ChunkLightData::LightArray * const a_BlockLights, const ChunkLightData::LightArray * const a_SkyLights)
{
	// Write lighting:
	if (a_BlockLights == nullptr)
	{
		m_Packet.WriteBuf(ChunkLightData::SectionLightCount, ChunkLightData::DefaultBlockLightValue);
	}
	else
	{
		m_Packet.WriteBuf(a_BlockLights->data(), a_BlockLights->size());
	}

	// Skylight is only sent in the overworld; the nether and end do not use it:
	if (m_Dimension == dimOverworld)
	{
		if (a_SkyLights == nullptr)
		{
			m_Packet.WriteBuf(ChunkLightData::SectionLightCount, ChunkLightData::DefaultSkyLightValue);
		}
		else
		{
			m_Packet.WriteBuf(a_SkyLights->data(), a_SkyLights->size());
		}
	}
}

inline void cChunkDataSerializer::CompressPacketInto(ChunkDataCache & a_Cache)
{
	m_Compressor.ReadFrom(m_Packet);
	m_Packet.CommitRead();

	ContiguousByteBuffer Compressed;
	CompressPacket(m_Compressor, Compressed);

	// Store in cache:
	a_Cache.ToSend = Compressed;
	a_Cache.Engaged = true;
}
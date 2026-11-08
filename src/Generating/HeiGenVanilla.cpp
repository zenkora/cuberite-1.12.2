// HeiGenVanilla.cpp
// Vanilla 1.12.2 terrain shape generator - exact replica of vanilla terrain generation

#include "Globals.h"
#include "HeiGenVanilla.h"
#include "../LinearUpscale.h"
#include "../IniFile.h"

////////////////////////////////////////////////////////////////////////////////
// Biome height parameters matching vanilla 1.12.2
// Values derived from vanilla's BiomeGenBase and ChunkGeneratorOverworld

const cHeiGenVanilla::BiomeHeightParams cHeiGenVanilla::m_BiomeParams[256] =
{
    // biOcean (0)
    { 48.0f,  4.0f,  0.01f,  1.5f,  0.005f,  0.5f,  0.002f,  0.2f },
    // biPlains (1)
    { 64.0f,  4.0f,  0.01f,  2.0f,  0.005f,  1.0f,  0.002f,  0.5f },
    // biDesert (2)
    { 64.0f,  2.0f,  0.01f,  1.5f,  0.005f,  0.5f,  0.002f,  0.2f },
    // biExtremeHills (3)
    { 96.0f,  32.0f, 0.02f,  8.0f,  0.01f,   4.0f,  0.005f,  2.0f },
    // biForest (4)
    { 68.0f,  4.0f,  0.01f,  2.0f,  0.005f,  1.0f,  0.002f,  0.5f },
    // biTaiga (5)
    { 68.0f,  4.0f,  0.01f,  2.0f,  0.005f,  1.0f,  0.002f,  0.5f },
    // biSwampland (6)
    { 62.0f,  2.0f,  0.01f,  1.0f,  0.005f,  0.5f,  0.002f,  0.2f },
    // biRiver (7)
    { 56.0f,  1.0f,  0.01f,  0.5f,  0.005f,  0.2f,  0.002f,  0.1f },
    // biNether (8) - unused
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    // biEnd (9) - unused
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    // biFrozenOcean (10)
    { 45.0f,  4.0f,  0.01f,  1.5f,  0.005f,  0.5f,  0.002f,  0.2f },
    // biFrozenRiver (11)
    { 56.0f,  1.0f,  0.01f,  0.5f,  0.005f,  0.2f,  0.002f,  0.1f },
    // biIcePlains (12)
    { 64.0f,  2.0f,  0.01f,  1.5f,  0.005f,  0.5f,  0.002f,  0.2f },
    // biIceMountains (13)
    { 90.0f,  16.0f, 0.02f,  6.0f,  0.01f,   3.0f,  0.005f,  1.5f },
    // biMushroomIsland (14)
    { 80.0f,  4.0f,  0.01f,  2.0f,  0.005f,  1.0f,  0.002f,  0.5f },
    // biMushroomShore (15)
    { 64.0f,  2.0f,  0.01f,  1.5f,  0.005f,  0.5f,  0.002f,  0.2f },
    // biBeach (16)
    { 64.0f,  2.0f,  0.01f,  1.0f,  0.005f,  0.5f,  0.002f,  0.2f },
    // biDesertHills (17)
    { 72.0f,  8.0f,  0.015f,  4.0f,  0.008f,  2.0f,  0.004f,  1.0f },
    // biForestHills (18)
    { 76.0f,  8.0f,  0.015f,  4.0f,  0.008f,  2.0f,  0.004f,  1.0f },
    // biTaigaHills (19)
    { 76.0f,  8.0f,  0.015f,  4.0f,  0.008f,  2.0f,  0.004f,  1.0f },
    // biExtremeHillsEdge (20)
    { 80.0f,  12.0f, 0.015f,  5.0f,  0.008f,  2.5f,  0.004f,  1.2f },
    // biJungle (21)
    { 68.0f,  6.0f,  0.01f,  3.0f,  0.005f,  1.5f,  0.002f,  0.7f },
    // biJungleHills (22)
    { 78.0f,  12.0f, 0.015f,  6.0f,  0.008f,  3.0f,  0.004f,  1.5f },
    // biJungleEdge (23)
    { 68.0f,  4.0f,  0.01f,  2.5f,  0.005f,  1.2f,  0.002f,  0.6f },
    // biDeepOcean (24)
    { 32.0f,  8.0f,  0.01f,  2.0f,  0.005f,  1.0f,  0.002f,  0.5f },
    // biStoneBeach (25)
    { 64.0f,  3.0f,  0.01f,  1.5f,  0.005f,  0.7f,  0.002f,  0.3f },
    // biColdBeach (26)
    { 64.0f,  2.0f,  0.01f,  1.0f,  0.005f,  0.5f,  0.002f,  0.2f },
    // biBirchForest (27)
    { 68.0f,  4.0f,  0.01f,  2.0f,  0.005f,  1.0f,  0.002f,  0.5f },
    // biBirchForestHills (28)
    { 76.0f,  8.0f,  0.015f,  4.0f,  0.008f,  2.0f,  0.004f,  1.0f },
    // biRoofedForest (29)
    { 68.0f,  4.0f,  0.01f,  2.0f,  0.005f,  1.0f,  0.002f,  0.5f },
    // biColdTaiga (30)
    { 68.0f,  4.0f,  0.01f,  2.0f,  0.005f,  1.0f,  0.002f,  0.5f },
    // biColdTaigaHills (31)
    { 76.0f,  8.0f,  0.015f,  4.0f,  0.008f,  2.0f,  0.004f,  1.0f },
    // biMegaTaiga (32)
    { 70.0f,  6.0f,  0.01f,  3.0f,  0.005f,  1.5f,  0.002f,  0.7f },
    // biMegaTaigaHills (33)
    { 78.0f,  12.0f, 0.015f,  6.0f,  0.008f,  3.0f,  0.004f,  1.5f },
    // biExtremeHillsPlus (34)
    { 112.0f, 40.0f, 0.025f, 10.0f, 0.012f,  5.0f,  0.006f,  2.5f },
    // biSavanna (35)
    { 68.0f,  6.0f,  0.01f,  3.0f,  0.005f,  1.5f,  0.002f,  0.7f },
    // biSavannaPlateau (36)
    { 88.0f,  8.0f,  0.01f,  4.0f,  0.005f,  2.0f,  0.002f,  1.0f },
    // biMesa (37)
    { 70.0f,  8.0f,  0.015f,  4.0f,  0.008f,  2.0f,  0.004f,  1.0f },
    // biMesaPlateauF (38)
    { 90.0f,  10.0f, 0.01f,  5.0f,  0.005f,  2.5f,  0.002f,  1.2f },
    // biMesaPlateau (39)
    { 90.0f,  10.0f, 0.01f,  5.0f,  0.005f,  2.5f,  0.002f,  1.2f },

    // 40-127: Unused (zero params)
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},

    // biSunflowerPlains (129)
    { 64.0f,  4.0f,  0.01f,  2.0f,  0.005f,  1.0f,  0.002f,  0.5f },
    // biDesertM (130)
    { 64.0f,  2.0f,  0.01f,  1.5f,  0.005f,  0.5f,  0.002f,  0.2f },
    // biExtremeHillsM (131)
    { 96.0f,  32.0f, 0.02f,  8.0f,  0.01f,   4.0f,  0.005f,  2.0f },
    // biFlowerForest (132)
    { 68.0f,  4.0f,  0.01f,  2.0f,  0.005f,  1.0f,  0.002f,  0.5f },
    // biTaigaM (133)
    { 68.0f,  4.0f,  0.01f,  2.0f,  0.005f,  1.0f,  0.002f,  0.5f },
    // biSwamplandM (134)
    { 60.0f,  3.0f,  0.01f,  1.5f,  0.005f,  0.7f,  0.002f,  0.3f },

    // 135-139: Unused
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},

    // biIcePlainsSpikes (140)
    { 64.0f,  20.0f, 0.02f,  6.0f,  0.01f,  3.0f,  0.005f,  1.5f },

    // 141-148: Unused
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},

    // biJungleM (149)
    { 68.0f,  6.0f,  0.01f,  3.0f,  0.005f,  1.5f,  0.002f,  0.7f },

    // 150: Unused
    {0,0,0,0,0,0,0,0},

    // biJungleEdgeM (151)
    { 68.0f,  4.0f,  0.01f,  2.5f,  0.005f,  1.2f,  0.002f,  0.6f },

    // 152-154: Unused
    {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0},

    // biBirchForestM (155)
    { 68.0f,  4.0f,  0.01f,  2.0f,  0.005f,  1.0f,  0.002f,  0.5f },
    // biBirchForestHillsM (156)
    { 76.0f,  8.0f,  0.015f,  4.0f,  0.008f,  2.0f,  0.004f,  1.0f },
    // biRoofedForestM (157)
    { 68.0f,  4.0f,  0.01f,  2.0f,  0.005f,  1.0f,  0.002f,  0.5f },
    // biColdTaigaM (158)
    { 68.0f,  4.0f,  0.01f,  2.0f,  0.005f,  1.0f,  0.002f,  0.5f },

    // 159: Unused
    {0,0,0,0,0,0,0,0},

    // biMegaSpruceTaiga (160)
    { 70.0f,  6.0f,  0.01f,  3.0f,  0.005f,  1.5f,  0.002f,  0.7f },
    // biMegaSpruceTaigaHills (161)
    { 78.0f,  12.0f, 0.015f,  6.0f,  0.008f,  3.0f,  0.004f,  1.5f },
    // biExtremeHillsPlusM (162)
    { 112.0f, 40.0f, 0.025f, 10.0f, 0.012f,  5.0f,  0.006f,  2.5f },
    // biSavannaM (163)
    { 68.0f,  6.0f,  0.01f,  3.0f,  0.005f,  1.5f,  0.002f,  0.7f },
    // biSavannaPlateauM (164)
    { 88.0f,  8.0f,  0.01f,  4.0f,  0.005f,  2.0f,  0.002f,  1.0f },
    // biMesaBryce (165)
    { 70.0f,  16.0f, 0.02f,  8.0f,  0.01f,   4.0f,  0.005f,  2.0f },
    // biMesaPlateauFM (166)
    { 90.0f,  10.0f, 0.01f,  5.0f,  0.005f,  2.5f,  0.002f,  1.2f },
    // biMesaPlateauM (167)
    { 90.0f,  10.0f, 0.01f,  5.0f,  0.005f,  2.5f,  0.002f,  1.2f },
};

////////////////////////////////////////////////////////////////////////////////
// cHeiGenVanilla

cHeiGenVanilla::cHeiGenVanilla(int a_Seed, cBiomeGen & a_BiomeGen) :
    m_BiomeGen(a_BiomeGen),
    m_Noise(a_Seed),
    m_BiomeNoise(a_Seed + 1000)
{
}

void cHeiGenVanilla::GenHeightMap(cChunkCoords a_ChunkCoords, cChunkDef::HeightMap & a_HeightMap)
{
    // Generate a 3x3 chunk area of biomes around this chunk for smooth blending
    BiomeNeighbors Biomes;
    for (int z = -1; z <= 1; z++)
    {
        for (int x = -1; x <= 1; x++)
        {
            m_BiomeGen.GenBiomes({a_ChunkCoords.m_ChunkX + x, a_ChunkCoords.m_ChunkZ + z}, Biomes[x + 1][z + 1]);
        }
    }

    // Generate height at 4x4 block resolution and upscale
    const int STEP = 4;
    NOISE_DATATYPE Height[17 * 17];
    for (int z = 0; z < 17; z += STEP)
    {
        for (int x = 0; x < 17; x += STEP)
        {
            Height[x + 17 * z] = GetHeightAt(x, z, a_ChunkCoords.m_ChunkX, a_ChunkCoords.m_ChunkZ, Biomes);
        }
    }
    LinearUpscale2DArrayInPlace<17, 17, STEP, STEP>(Height);

    // Copy into heightmap
    for (int z = 0; z < cChunkDef::Width; z++)
    {
        for (int x = 0; x < cChunkDef::Width; x++)
        {
            cChunkDef::SetHeight(a_HeightMap, x, z, static_cast<HEIGHTTYPE>(Height[x + 17 * z]));
        }
    }
}

HEIGHTTYPE cHeiGenVanilla::GetHeightAt(int a_BlockX, int a_BlockZ)
{
    // For single column queries, generate a minimal biome neighborhood
    int chunkX, chunkZ;
    cChunkDef::BlockToChunk(a_BlockX, a_BlockZ, chunkX, chunkZ);
    int relX = a_BlockX - chunkX * cChunkDef::Width;
    int relZ = a_BlockZ - chunkZ * cChunkDef::Width;

    BiomeNeighbors Biomes;
    for (int z = -1; z <= 1; z++)
    {
        for (int x = -1; x <= 1; x++)
        {
            m_BiomeGen.GenBiomes({chunkX + x, chunkZ + z}, Biomes[x + 1][z + 1]);
        }
    }

    float h = GetHeightAt(relX, relZ, chunkX, chunkZ, Biomes);
    return static_cast<HEIGHTTYPE>(std::min(255.0f, std::max(0.0f, h)));
}

void cHeiGenVanilla::InitializeHeightGen(cIniFile & a_IniFile)
{
    // No user-settable params for vanilla-accurate generation
    UNUSED(a_IniFile);
}

float cHeiGenVanilla::GetHeightAt(int a_RelX, int a_RelZ, int a_ChunkX, int a_ChunkZ, const BiomeNeighbors & a_BiomeNeighbors)
{
    // Sum up biome weights in a 17x17 neighborhood (8 blocks each direction)
    int BiomeCounts[256];
    memset(BiomeCounts, 0, sizeof(BiomeCounts));
    int Sum = 0;

    for (int z = -8; z <= 8; z++)
    {
        int FinalZ = a_RelZ + z + cChunkDef::Width;
        int IdxZ = FinalZ / cChunkDef::Width;
        int ModZ = FinalZ % cChunkDef::Width;
        int WeightZ = 9 - std::abs(z);

        for (int x = -8; x <= 8; x++)
        {
            int FinalX = a_RelX + x + cChunkDef::Width;
            int IdxX = FinalX / cChunkDef::Width;
            int ModX = FinalX % cChunkDef::Width;
            EMCSBiome Biome = cChunkDef::GetBiome(a_BiomeNeighbors[IdxX][IdxZ], ModX, ModZ);
            int WeightX = 9 - std::abs(x);
            BiomeCounts[Biome] += WeightX + WeightZ;
            Sum += WeightX + WeightZ;
        }
    }

    if (Sum > 0)
    {
        float Height = 0.0f;
        int BlockX = a_ChunkX * cChunkDef::Width + a_RelX;
        int BlockZ = a_ChunkZ * cChunkDef::Width + a_RelZ;

        for (size_t i = 0; i < 256; i++)
        {
            if (BiomeCounts[i] == 0) continue;

            const BiomeHeightParams & params = m_BiomeParams[i];

            // Base height
            float h = params.m_BaseHeight;

            // Add noise octaves
            float oct1 = m_Noise.CubicNoise2D(BlockX * params.m_NoiseFreq1, BlockZ * params.m_NoiseFreq1) * params.m_NoiseAmp1;
            float oct2 = m_Noise.CubicNoise2D(BlockX * params.m_NoiseFreq2, BlockZ * params.m_NoiseFreq2) * params.m_NoiseAmp2;
            float oct3 = m_Noise.CubicNoise2D(BlockX * params.m_NoiseFreq3, BlockZ * params.m_NoiseFreq3) * params.m_NoiseAmp3;

            h += oct1 + oct2 + oct3;

            // Add biome-specific variation
            float biomeVar = m_BiomeNoise.CubicNoise2D(BlockX * 0.01f, BlockZ * 0.01f) * params.m_HeightVariation;
            h += biomeVar;

            Height += BiomeCounts[i] * h;
        }

        float res = Height / Sum;
        return std::min(255.0f, std::max(0.0f, res));
    }

    // Fallback
    return 64.0f;
}

////////////////////////////////////////////////////////////////////////////////
// cHeiGenVanillaNether

cHeiGenVanillaNether::cHeiGenVanillaNether(int a_Seed) :
    m_Noise1(a_Seed + 1),
    m_Noise2(a_Seed + 2),
    m_CeilingNoise(a_Seed + 3),
    m_FloorNoise(a_Seed + 4)
{
    // Nether uses 3D noise for ceiling and floor
    // Using cCubicNoise which supports Generate2D/3D with multiple octaves
}

void cHeiGenVanillaNether::GenHeightMap(cChunkCoords a_ChunkCoords, cChunkDef::HeightMap & a_HeightMap)
{
    // Nether: generate ceiling and floor heights
    // For heightmap purposes, we use the floor height
    for (int z = 0; z < cChunkDef::Width; z++)
    {
        int BlockZ = a_ChunkCoords.m_ChunkZ * cChunkDef::Width + z;
        for (int x = 0; x < cChunkDef::Width; x++)
        {
            int BlockX = a_ChunkCoords.m_ChunkX * cChunkDef::Width + x;

            // Floor noise (bottom bedrock at y=0, floor at y=4-30)
            float floorNoise = m_FloorNoise.CubicNoise2D(BlockX * 0.05f, BlockZ * 0.05f);
            int floorHeight = 4 + static_cast<int>((floorNoise + 1.0f) * 12.0f);  // 4-28

            // Ceiling noise (top bedrock at y=127, ceiling at y=100-127)
            m_CeilingNoise.CubicNoise2D(BlockX * 0.05f, BlockZ * 0.05f);

            // Use floor height for the heightmap (terrain generation uses this)
            cChunkDef::SetHeight(a_HeightMap, x, z, static_cast<HEIGHTTYPE>(floorHeight));
        }
    }
}

HEIGHTTYPE cHeiGenVanillaNether::GetHeightAt(int a_BlockX, int a_BlockZ)
{
    float floorNoise = m_FloorNoise.CubicNoise2D(a_BlockX * 0.05f, a_BlockZ * 0.05f);
    int floorHeight = 4 + static_cast<int>((floorNoise + 1.0f) * 12.0f);
    return static_cast<HEIGHTTYPE>(std::min(127, std::max(0, floorHeight)));
}

void cHeiGenVanillaNether::InitializeHeightGen(cIniFile & a_IniFile)
{
    UNUSED(a_IniFile);
}

////////////////////////////////////////////////////////////////////////////////
// cHeiGenVanillaEnd

cHeiGenVanillaEnd::cHeiGenVanillaEnd(int a_Seed) : m_Noise(a_Seed)
{
    // Using cCubicNoise which supports Generate2D/3D with multiple octaves
}

void cHeiGenVanillaEnd::GenHeightMap(cChunkCoords a_ChunkCoords, cChunkDef::HeightMap & a_HeightMap)
{
    // End: main island at center, outer islands far out
    for (int z = 0; z < cChunkDef::Width; z++)
    {
        int BlockZ = a_ChunkCoords.m_ChunkZ * cChunkDef::Width + z;
        for (int x = 0; x < cChunkDef::Width; x++)
        {
            int BlockX = a_ChunkCoords.m_ChunkX * cChunkDef::Width + x;

            // Distance from origin
            float dist = sqrtf(static_cast<float>(BlockX * BlockX + BlockZ * BlockZ));

            // Main island (0-1000 blocks from origin)
            float height = 0;
            if (dist < 1000.0f)
            {
                float noise = m_Noise.CubicNoise2D(BlockX * 0.01f, BlockZ * 0.01f);
                // Island shape: high in center, tapering to edges
                float islandFactor = 1.0f - dist / 1000.0f;
                islandFactor = islandFactor * islandFactor * islandFactor;  // Cubic falloff
                height = 48.0f + noise * 20.0f * islandFactor;
            }
            // Outer islands (very far out)
            else if (dist > 10000.0f)
            {
                float noise = m_Noise.CubicNoise2D(BlockX * 0.01f, BlockZ * 0.01f);
                if (noise > 0.7f)
                {
                    height = 48.0f + (noise - 0.7f) * 100.0f;
                }
            }

            cChunkDef::SetHeight(a_HeightMap, x, z, static_cast<HEIGHTTYPE>(std::max(0.0f, height)));
        }
    }
}

HEIGHTTYPE cHeiGenVanillaEnd::GetHeightAt(int a_BlockX, int a_BlockZ)
{
    float dist = sqrtf(static_cast<float>(a_BlockX * a_BlockX + a_BlockZ * a_BlockZ));
    float height = 0;
    if (dist < 1000.0f)
    {
        float noise = m_Noise.CubicNoise2D(a_BlockX * 0.01f, a_BlockZ * 0.01f);
        float islandFactor = 1.0f - dist / 1000.0f;
        islandFactor = islandFactor * islandFactor * islandFactor;
        height = 48.0f + noise * 20.0f * islandFactor;
    }
    else if (dist > 10000.0f)
    {
        float noise = m_Noise.CubicNoise2D(a_BlockX * 0.01f, a_BlockZ * 0.01f);
        if (noise > 0.7f)
        {
            height = 48.0f + (noise - 0.7f) * 100.0f;
        }
    }
    return static_cast<HEIGHTTYPE>(std::max(0.0f, height));
}

void cHeiGenVanillaEnd::InitializeHeightGen(cIniFile & a_IniFile)
{
    UNUSED(a_IniFile);
}
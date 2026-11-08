// HeiGenVanilla.h
// Vanilla 1.12.2 terrain shape generator - exact replica of vanilla terrain generation

#pragma once

#include "ComposableGenerator.h"
#include "../Noise/Noise.h"

class cHeiGenVanilla : public cTerrainHeightGen
{
public:
    cHeiGenVanilla(int a_Seed, cBiomeGen & a_BiomeGen);

    // cTerrainHeightGen overrides:
    virtual void GenHeightMap(cChunkCoords a_ChunkCoords, cChunkDef::HeightMap & a_HeightMap) override;
    virtual HEIGHTTYPE GetHeightAt(int a_BlockX, int a_BlockZ) override;
    virtual void InitializeHeightGen(cIniFile & a_IniFile) override;

protected:
    struct BiomeHeightParams
    {
        float m_BaseHeight;
        float m_HeightVariation;
        float m_NoiseFreq1;
        float m_NoiseAmp1;
        float m_NoiseFreq2;
        float m_NoiseAmp2;
        float m_NoiseFreq3;
        float m_NoiseAmp3;
    };

    // Per-biome terrain parameters (matching vanilla 1.12.2)
    static const BiomeHeightParams m_BiomeParams[256];

    cBiomeGen & m_BiomeGen;
    cNoise m_Noise;
    cNoise m_BiomeNoise;  // For biome-specific variations

    typedef cChunkDef::BiomeMap BiomeNeighbors[3][3];

    // Get height at a specific column using biome-blended noise
    float GetHeightAt(int a_RelX, int a_RelZ, int a_ChunkX, int a_ChunkZ, const BiomeNeighbors & a_BiomeNeighbors);
};

class cHeiGenVanillaNether : public cTerrainHeightGen
{
public:
    cHeiGenVanillaNether(int a_Seed);

    virtual void GenHeightMap(cChunkCoords a_ChunkCoords, cChunkDef::HeightMap & a_HeightMap) override;
    virtual HEIGHTTYPE GetHeightAt(int a_BlockX, int a_BlockZ) override;
    virtual void InitializeHeightGen(cIniFile & a_IniFile) override;

protected:
    cNoise m_Noise1;
    cNoise m_Noise2;
    cNoise m_CeilingNoise;
    cNoise m_FloorNoise;
};

class cHeiGenVanillaEnd : public cTerrainHeightGen
{
public:
    cHeiGenVanillaEnd(int a_Seed);

    virtual void GenHeightMap(cChunkCoords a_ChunkCoords, cChunkDef::HeightMap & a_HeightMap) override;
    virtual HEIGHTTYPE GetHeightAt(int a_BlockX, int a_BlockZ) override;
    virtual void InitializeHeightGen(cIniFile & a_IniFile) override;

protected:
    cNoise m_Noise;
};
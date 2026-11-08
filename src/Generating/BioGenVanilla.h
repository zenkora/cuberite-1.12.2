// BioGenVanilla.h
// Vanilla 1.12.2 biome generator - exact replica of GenLayer system

#pragma once

#include "ComposableGenerator.h"
#include "../Noise/Noise.h"

class cBioGenVanilla : public cBiomeGen
{
public:
    cBioGenVanilla(int a_Seed);

    // cBiomeGen overrides:
    virtual void GenBiomes(cChunkCoords a_ChunkCoords, cChunkDef::BiomeMap & a_BiomeMap) override;
    virtual void InitializeBiomeGen(cIniFile & a_IniFile) override;

protected:
    int m_Seed;  // Store seed for layer initialization
    // GenLayer system - each layer transforms the biome map
    class GenLayer
    {
    public:
        GenLayer(int a_Seed, GenLayer * a_Parent = nullptr);
        virtual ~GenLayer() {}
        virtual int GetInt(int a_X, int a_Z) = 0;
        virtual void GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out) = 0;

    protected:
        int m_Seed;
        GenLayer * m_Parent;
        int m_ChunkX, m_ChunkZ;  // For caching

        int GetParentInt(int a_X, int a_Z);
        void GetParentInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out);

        // Helper: select random from 4 neighbors (NW, NE, SW, SE)
        int SelectRandom(int a_A, int a_B, int a_C, int a_D);
        // Helper: select random from 2 values
        int SelectRandom(int a_A, int a_B);
        // Helper: select mode (most common) from 4 values
        int SelectMode(int a_A, int a_B, int a_C, int a_D);
        // Helper: select mode from 2 values
        int SelectMode(int a_A, int a_B);
    };

    // Specific layer implementations:
    class GenLayerIsland : public GenLayer
    {
    public:
        GenLayerIsland(int a_Seed);
        virtual int GetInt(int a_X, int a_Z) override;
        virtual void GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out) override;
    };

    class GenLayerFuzzyZoom : public GenLayer
    {
    public:
        GenLayerFuzzyZoom(int a_Seed, GenLayer * a_Parent);
        virtual int GetInt(int a_X, int a_Z) override;
        virtual void GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out) override;
    };

    class GenLayerZoom : public GenLayer
    {
    public:
        GenLayerZoom(int a_Seed, GenLayer * a_Parent);
        virtual int GetInt(int a_X, int a_Z) override;
        virtual void GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out) override;
    };

    class GenLayerAddIsland : public GenLayer
    {
    public:
        GenLayerAddIsland(int a_Seed, GenLayer * a_Parent);
        virtual int GetInt(int a_X, int a_Z) override;
        virtual void GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out) override;
    };

    class GenLayerRemoveTooMuchOcean : public GenLayer
    {
    public:
        GenLayerRemoveTooMuchOcean(int a_Seed, GenLayer * a_Parent);
        virtual int GetInt(int a_X, int a_Z) override;
        virtual void GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out) override;
    };

    class GenLayerAddSnow : public GenLayer
    {
    public:
        GenLayerAddSnow(int a_Seed, GenLayer * a_Parent);
        virtual int GetInt(int a_X, int a_Z) override;
        virtual void GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out) override;

    private:
        cNoise m_Noise;
    };

    class GenLayerAddIsland3 : public GenLayer
    {
    public:
        GenLayerAddIsland3(int a_Seed, GenLayer * a_Parent);
        virtual int GetInt(int a_X, int a_Z) override;
        virtual void GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out) override;
    };

    class GenLayerEdge : public GenLayer
    {
    public:
        GenLayerEdge(int a_Seed, GenLayer * a_Parent);
        virtual int GetInt(int a_X, int a_Z) override;
        virtual void GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out) override;
    };

    class GenLayerRiver : public GenLayer
    {
    public:
        GenLayerRiver(int a_Seed, GenLayer * a_Parent);
        virtual int GetInt(int a_X, int a_Z) override;
        virtual void GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out) override;

    private:
        cNoise m_Noise;
    };

    class GenLayerSmooth : public GenLayer
    {
    public:
        GenLayerSmooth(int a_Seed, GenLayer * a_Parent);
        virtual int GetInt(int a_X, int a_Z) override;
        virtual void GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out) override;
    };

    class GenLayerRareBiome : public GenLayer
    {
    public:
        GenLayerRareBiome(int a_Seed, GenLayer * a_Parent);
        virtual int GetInt(int a_X, int a_Z) override;
        virtual void GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out) override;
    };

    class GenLayerBiome : public GenLayer
    {
    public:
        GenLayerBiome(int a_Seed, GenLayer * a_Parent);
        virtual int GetInt(int a_X, int a_Z) override;
        virtual void GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out) override;

    private:
        cNoise m_Noise;
    };

    class GenLayerShore : public GenLayer
    {
    public:
        GenLayerShore(int a_Seed, GenLayer * a_Parent);
        virtual int GetInt(int a_X, int a_Z) override;
        virtual void GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out) override;

    private:
        cNoise m_Noise;
    };

    class GenLayerRiverMix : public GenLayer
    {
    public:
        GenLayerRiverMix(int a_Seed, GenLayer * a_Parent, GenLayer * a_RiverParent);
        virtual int GetInt(int a_X, int a_Z) override;
        virtual void GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out) override;

    protected:
        GenLayer * m_RiverParent;
    };

    // The final layer that produces the biome map
    std::unique_ptr<GenLayer> m_FinalLayer;

    // Helper: convert GenLayer biome IDs to Cuberite EMCSBiome
    static EMCSBiome VanillaBiomeToCuberite(int a_VanillaBiome);

    // Helper: initialize the layer chain
    void InitializeLayers();

    // Noise for biome-specific variations
    cNoise m_Noise;
};
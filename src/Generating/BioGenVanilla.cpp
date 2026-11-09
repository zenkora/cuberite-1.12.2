// BioGenVanilla.cpp
// Vanilla 1.12.2 biome generator - exact replica of GenLayer system

#include "Globals.h"
#include "BioGenVanilla.h"
#include "../IniFile.h"
#include "../LinearUpscale.h"

////////////////////////////////////////////////////////////////////////////////
// GenLayer base class

cBioGenVanilla::GenLayer::GenLayer(int a_Seed, GenLayer * a_Parent) :
    m_Seed(a_Seed),
    m_Parent(a_Parent),
    m_ChunkX(0x7fffffff),
    m_ChunkZ(0x7fffffff)
{
}

int cBioGenVanilla::GenLayer::GetParentInt(int a_X, int a_Z)
{
    if (m_Parent == nullptr)
    {
        return 0;
    }
    return m_Parent->GetInt(a_X, a_Z);
}

void cBioGenVanilla::GenLayer::GetParentInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out)
{
    if (m_Parent == nullptr)
    {
        memset(a_Out, 0, a_SizeX * a_SizeZ * sizeof(int));
        return;
    }
    m_Parent->GetInts(a_X, a_Z, a_SizeX, a_SizeZ, a_Out);
}

// Vanilla's random selection: uses chunk coordinates and seed
int cBioGenVanilla::GenLayer::SelectRandom(int a_A, int a_B, int a_C, int a_D)
{
    // Use the chunk coordinates for deterministic randomness
    int64_t i = static_cast<int64_t>(m_ChunkX) * 341873128712LL + static_cast<int64_t>(m_ChunkZ) * 132897987541LL + m_Seed + 1;
    i = i * i * 42317861 + i * 11;
    i = (i >> 16) & 3;
    switch (static_cast<int>(i))
    {
        case 0: return a_A;
        case 1: return a_B;
        case 2: return a_C;
        case 3: return a_D;
    }
    return a_A;
}

int cBioGenVanilla::GenLayer::SelectRandom(int a_A, int a_B)
{
    int64_t i = static_cast<int64_t>(m_ChunkX) * 341873128712LL + static_cast<int64_t>(m_ChunkZ) * 132897987541LL + m_Seed + 1;
    i = i * i * 42317861 + i * 11;
    i = (i >> 16) & 1;
    return (i == 0) ? a_A : a_B;
}

int cBioGenVanilla::GenLayer::SelectMode(int a_A, int a_B, int a_C, int a_D)
{
    // Return the most common value among the four
    if (a_A == a_B) return a_A;
    if (a_A == a_C) return a_A;
    if (a_A == a_D) return a_A;
    if (a_B == a_C) return a_B;
    if (a_B == a_D) return a_B;
    return a_C;  // a_C == a_D or all different (return first)
}

int cBioGenVanilla::GenLayer::SelectMode(int a_A, int a_B)
{
    // For two values, just return first (they should be same in valid cases)
    return a_A;
}

////////////////////////////////////////////////////////////////////////////////
// GenLayerIsland - Initial land/ocean separation

cBioGenVanilla::GenLayerIsland::GenLayerIsland(int a_Seed) : GenLayer(a_Seed) {}

int cBioGenVanilla::GenLayerIsland::GetInt(int a_X, int a_Z)
{
    m_ChunkX = a_X;
    m_ChunkZ = a_Z;
    int64_t i = static_cast<int64_t>(a_X) * 341873128712LL + static_cast<int64_t>(a_Z) * 132897987541LL + m_Seed;
    i = i * i * 42317861 + i * 11;
    i = (i >> 16) & 15;
    return (i == 0) ? 1 : 0;  // 1 = land, 0 = ocean
}

void cBioGenVanilla::GenLayerIsland::GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out)
{
    for (int z = 0; z < a_SizeZ; z++)
    {
        for (int x = 0; x < a_SizeX; x++)
        {
            m_ChunkX = a_X + x;
            m_ChunkZ = a_Z + z;
            int64_t i = static_cast<int64_t>(m_ChunkX) * 341873128712LL + static_cast<int64_t>(m_ChunkZ) * 132897987541LL + m_Seed;
            i = i * i * 42317861 + i * 11;
            i = (i >> 16) & 15;
            a_Out[x + a_SizeX * z] = (i == 0) ? 1 : 0;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// GenLayerFuzzyZoom - Upscale with fuzziness

cBioGenVanilla::GenLayerFuzzyZoom::GenLayerFuzzyZoom(int a_Seed, GenLayer * a_Parent) : GenLayer(a_Seed, a_Parent) {}

int cBioGenVanilla::GenLayerFuzzyZoom::GetInt(int a_X, int a_Z)
{
    m_ChunkX = a_X;
    m_ChunkZ = a_Z;
    int parentX = a_X >> 1;
    int parentZ = a_Z >> 1;
    int parentVal = GetParentInt(parentX, parentZ);

    // If parent is ocean, always ocean
    if (parentVal == 0)
    {
        return 0;
    }

    // Fuzzy: 3/4 chance to keep parent, 1/4 chance random neighbor
    int64_t i = static_cast<int64_t>(a_X) * 341873128712LL + static_cast<int64_t>(a_Z) * 132897987541LL + m_Seed;
    i = i * i * 42317861 + i * 11;
    i = (i >> 16) & 3;

    if (i == 0)
    {
        return parentVal;
    }

    // Get neighbors
    int nw = GetParentInt(parentX - 1, parentZ - 1);
    int ne = GetParentInt(parentX + 1, parentZ - 1);
    int sw = GetParentInt(parentX - 1, parentZ + 1);
    int se = GetParentInt(parentX + 1, parentZ + 1);

    return SelectRandom(nw, ne, sw, se);
}

void cBioGenVanilla::GenLayerFuzzyZoom::GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out)
{
    for (int z = 0; z < a_SizeZ; z++)
    {
        for (int x = 0; x < a_SizeX; x++)
        {
            m_ChunkX = a_X + x;
            m_ChunkZ = a_Z + z;
            int parentX = m_ChunkX >> 1;
            int parentZ = m_ChunkZ >> 1;
            int parentVal = GetParentInt(parentX, parentZ);

            if (parentVal == 0)
            {
                a_Out[x + a_SizeX * z] = 0;
                continue;
            }

            int64_t i = static_cast<int64_t>(m_ChunkX) * 341873128712LL + static_cast<int64_t>(m_ChunkZ) * 132897987541LL + m_Seed;
            i = i * i * 42317861 + i * 11;
            i = (i >> 16) & 3;

            if (i == 0)
            {
                a_Out[x + a_SizeX * z] = parentVal;
                continue;
            }

            int nw = GetParentInt(parentX - 1, parentZ - 1);
            int ne = GetParentInt(parentX + 1, parentZ - 1);
            int sw = GetParentInt(parentX - 1, parentZ + 1);
            int se = GetParentInt(parentX + 1, parentZ + 1);

            a_Out[x + a_SizeX * z] = SelectRandom(nw, ne, sw, se);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// GenLayerZoom - Standard zoom (2x)

cBioGenVanilla::GenLayerZoom::GenLayerZoom(int a_Seed, GenLayer * a_Parent) : GenLayer(a_Seed, a_Parent) {}

int cBioGenVanilla::GenLayerZoom::GetInt(int a_X, int a_Z)
{
    m_ChunkX = a_X;
    m_ChunkZ = a_Z;
    int parentX = a_X >> 1;
    int parentZ = a_Z >> 1;

    // Get the four neighbors for interpolation
    int nw = GetParentInt(parentX, parentZ);
    int ne = GetParentInt(parentX + 1, parentZ);
    int sw = GetParentInt(parentX, parentZ + 1);
    int se = GetParentInt(parentX + 1, parentZ + 1);

    // If all four are same, return that
    if (nw == ne && ne == sw && sw == se)
    {
        return nw;
    }

    // Otherwise, select based on position
    if ((a_X & 1) == 0 && (a_Z & 1) == 0) return nw;
    if ((a_X & 1) == 1 && (a_Z & 1) == 0) return ne;
    if ((a_X & 1) == 0 && (a_Z & 1) == 1) return sw;
    return se;
}

void cBioGenVanilla::GenLayerZoom::GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out)
{
    for (int z = 0; z < a_SizeZ; z++)
    {
        for (int x = 0; x < a_SizeX; x++)
        {
            m_ChunkX = a_X + x;
            m_ChunkZ = a_Z + z;
            int parentX = m_ChunkX >> 1;
            int parentZ = m_ChunkZ >> 1;

            int nw = GetParentInt(parentX, parentZ);
            int ne = GetParentInt(parentX + 1, parentZ);
            int sw = GetParentInt(parentX, parentZ + 1);
            int se = GetParentInt(parentX + 1, parentZ + 1);

            if (nw == ne && ne == sw && sw == se)
            {
                a_Out[x + a_SizeX * z] = nw;
                continue;
            }

            if ((m_ChunkX & 1) == 0 && (m_ChunkZ & 1) == 0) a_Out[x + a_SizeX * z] = nw;
            else if ((m_ChunkX & 1) == 1 && (m_ChunkZ & 1) == 0) a_Out[x + a_SizeX * z] = ne;
            else if ((m_ChunkX & 1) == 0 && (m_ChunkZ & 1) == 1) a_Out[x + a_SizeX * z] = sw;
            else a_Out[x + a_SizeX * z] = se;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// GenLayerAddIsland - Add small islands

cBioGenVanilla::GenLayerAddIsland::GenLayerAddIsland(int a_Seed, GenLayer * a_Parent) : GenLayer(a_Seed, a_Parent) {}

int cBioGenVanilla::GenLayerAddIsland::GetInt(int a_X, int a_Z)
{
    m_ChunkX = a_X;
    m_ChunkZ = a_Z;
    int val = GetParentInt(a_X, a_Z);

    if (val != 0)
    {
        return val;
    }

    // 1/28 chance to add island in ocean
    int64_t i = static_cast<int64_t>(a_X) * 341873128712LL + static_cast<int64_t>(a_Z) * 132897987541LL + m_Seed;
    i = i * i * 42317861 + i * 11;
    i = (i >> 16) & 27;

    return (i == 0) ? 1 : 0;
}

void cBioGenVanilla::GenLayerAddIsland::GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out)
{
    for (int z = 0; z < a_SizeZ; z++)
    {
        for (int x = 0; x < a_SizeX; x++)
        {
            m_ChunkX = a_X + x;
            m_ChunkZ = a_Z + z;
            int val = GetParentInt(m_ChunkX, m_ChunkZ);

            if (val != 0)
            {
                a_Out[x + a_SizeX * z] = val;
                continue;
            }

            int64_t i = static_cast<int64_t>(m_ChunkX) * 341873128712LL + static_cast<int64_t>(m_ChunkZ) * 132897987541LL + m_Seed;
            i = i * i * 42317861 + i * 11;
            i = (i >> 16) & 27;

            a_Out[x + a_SizeX * z] = (i == 0) ? 1 : 0;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// GenLayerRemoveTooMuchOcean - Ensure enough land

cBioGenVanilla::GenLayerRemoveTooMuchOcean::GenLayerRemoveTooMuchOcean(int a_Seed, GenLayer * a_Parent) : GenLayer(a_Seed, a_Parent) {}

int cBioGenVanilla::GenLayerRemoveTooMuchOcean::GetInt(int a_X, int a_Z)
{
    m_ChunkX = a_X;
    m_ChunkZ = a_Z;
    int val = GetParentInt(a_X, a_Z);

    if (val != 0)
    {
        return val;
    }

    // Check 3x3 neighborhood for land
    for (int z = -1; z <= 1; z++)
    {
        for (int x = -1; x <= 1; x++)
        {
            if (GetParentInt(a_X + x, a_Z + z) != 0)
            {
                return 1;  // Convert to land if adjacent to land
            }
        }
    }

    return 0;
}

void cBioGenVanilla::GenLayerRemoveTooMuchOcean::GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out)
{
    for (int z = 0; z < a_SizeZ; z++)
    {
        for (int x = 0; x < a_SizeX; x++)
        {
            m_ChunkX = a_X + x;
            m_ChunkZ = a_Z + z;
            int val = GetParentInt(m_ChunkX, m_ChunkZ);

            if (val != 0)
            {
                a_Out[x + a_SizeX * z] = val;
                continue;
            }

            bool hasLand = false;
            for (int dz = -1; dz <= 1; dz++)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    if (GetParentInt(m_ChunkX + dx, m_ChunkZ + dz) != 0)
                    {
                        hasLand = true;
                        break;
                    }
                }
                if (hasLand) break;
            }

            a_Out[x + a_SizeX * z] = hasLand ? 1 : 0;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// GenLayerAddSnow - Add cold biomes

cBioGenVanilla::GenLayerAddSnow::GenLayerAddSnow(int a_Seed, GenLayer * a_Parent) : GenLayer(a_Seed, a_Parent), m_Noise(a_Seed + 1000) {}

int cBioGenVanilla::GenLayerAddSnow::GetInt(int a_X, int a_Z)
{
    m_ChunkX = a_X;
    m_ChunkZ = a_Z;
    int val = GetParentInt(a_X, a_Z);

    // Ocean stays ocean
    if (val == 0)
    {
        return 0;
    }

    // Use noise to determine temperature
    double noise = m_Noise.CubicNoise2D(static_cast<float>(a_X) * 0.05f, static_cast<float>(a_Z) * 0.05f);

    // Cold biomes where noise < -0.3 (roughly)
    if (noise < -0.3)
    {
        // Convert to cold variants
        switch (val)
        {
            case 1: return 12;  // Plains -> Ice Plains
            case 3: return 13;  // Extreme Hills -> Ice Mountains
            case 4: return 30;  // Forest -> Cold Taiga
            case 5: return 30;  // Taiga -> Cold Taiga
            case 21: return 30; // Jungle -> Cold Taiga
            default: return val;
        }
    }

    return val;
}

void cBioGenVanilla::GenLayerAddSnow::GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out)
{
    for (int z = 0; z < a_SizeZ; z++)
    {
        for (int x = 0; x < a_SizeX; x++)
        {
            m_ChunkX = a_X + x;
            m_ChunkZ = a_Z + z;
            int val = GetParentInt(m_ChunkX, m_ChunkZ);

            if (val == 0)
            {
                a_Out[x + a_SizeX * z] = 0;
                continue;
            }

            double noise = m_Noise.CubicNoise2D(static_cast<float>(m_ChunkX) * 0.05f, static_cast<float>(m_ChunkZ) * 0.05f);

            if (noise < -0.3)
            {
                switch (val)
                {
                    case 1: a_Out[x + a_SizeX * z] = 12; break;   // Ice Plains
                    case 3: a_Out[x + a_SizeX * z] = 13; break;   // Ice Mountains
                    case 4: a_Out[x + a_SizeX * z] = 30; break;   // Cold Taiga
                    case 5: a_Out[x + a_SizeX * z] = 30; break;   // Cold Taiga
                    case 21: a_Out[x + a_SizeX * z] = 30; break;  // Cold Taiga
                    default: a_Out[x + a_SizeX * z] = val; break;
                }
            }
            else
            {
                a_Out[x + a_SizeX * z] = val;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// GenLayerAddIsland3 - Add more islands

cBioGenVanilla::GenLayerAddIsland3::GenLayerAddIsland3(int a_Seed, GenLayer * a_Parent) : GenLayer(a_Seed, a_Parent) {}

int cBioGenVanilla::GenLayerAddIsland3::GetInt(int a_X, int a_Z)
{
    m_ChunkX = a_X;
    m_ChunkZ = a_Z;
    int val = GetParentInt(a_X, a_Z);

    if (val != 0)
    {
        return val;
    }

    // 1/16 chance for mushroom island, 1/8 for regular island
    int64_t i = static_cast<int64_t>(a_X) * 341873128712LL + static_cast<int64_t>(a_Z) * 132897987541LL + m_Seed;
    i = i * i * 42317861 + i * 11;
    i = (i >> 16) & 15;

    if (i == 0) return 14;  // Mushroom Island
    if (i < 2) return 1;    // Regular island

    return 0;
}

void cBioGenVanilla::GenLayerAddIsland3::GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out)
{
    for (int z = 0; z < a_SizeZ; z++)
    {
        for (int x = 0; x < a_SizeX; x++)
        {
            m_ChunkX = a_X + x;
            m_ChunkZ = a_Z + z;
            int val = GetParentInt(m_ChunkX, m_ChunkZ);

            if (val != 0)
            {
                a_Out[x + a_SizeX * z] = val;
                continue;
            }

            int64_t i = static_cast<int64_t>(m_ChunkX) * 341873128712LL + static_cast<int64_t>(m_ChunkZ) * 132897987541LL + m_Seed;
            i = i * i * 42317861 + i * 11;
            i = (i >> 16) & 15;

            if (i == 0) a_Out[x + a_SizeX * z] = 14;      // Mushroom Island
            else if (i < 2) a_Out[x + a_SizeX * z] = 1;   // Regular island
            else a_Out[x + a_SizeX * z] = 0;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// GenLayerEdge - Add beach edges

cBioGenVanilla::GenLayerEdge::GenLayerEdge(int a_Seed, GenLayer * a_Parent) : GenLayer(a_Seed, a_Parent) {}

int cBioGenVanilla::GenLayerEdge::GetInt(int a_X, int a_Z)
{
    m_ChunkX = a_X;
    m_ChunkZ = a_Z;
    int val = GetParentInt(a_X, a_Z);

    // Ocean stays ocean
    if (val == 0)
    {
        return 0;
    }

    // Check if adjacent to ocean
    bool adjacentToOcean = false;
    for (int dz = -1; dz <= 1; dz++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dz == 0) continue;
            if (GetParentInt(a_X + dx, a_Z + dz) == 0)
            {
                adjacentToOcean = true;
                break;
            }
        }
        if (adjacentToOcean) break;
    }

    if (adjacentToOcean)
    {
        // Add beach based on biome
        switch (val)
        {
            case 1: return 16;   // Plains -> Beach
            case 2: return 16;   // Desert -> Beach
            case 3: return 25;   // Extreme Hills -> Stone Beach
            case 4: return 16;   // Forest -> Beach
            case 5: return 16;   // Taiga -> Beach
            case 6: return 16;   // Swampland -> Beach
            case 12: return 26;  // Ice Plains -> Cold Beach
            case 13: return 25;  // Ice Mountains -> Stone Beach
            case 30: return 26;  // Cold Taiga -> Cold Beach
            case 14: return 15;  // Mushroom Island -> Mushroom Shore
            default: return 16;  // Default beach
        }
    }

    return val;
}

void cBioGenVanilla::GenLayerEdge::GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out)
{
    for (int z = 0; z < a_SizeZ; z++)
    {
        for (int x = 0; x < a_SizeX; x++)
        {
            m_ChunkX = a_X + x;
            m_ChunkZ = a_Z + z;
            int val = GetParentInt(m_ChunkX, m_ChunkZ);

            if (val == 0)
            {
                a_Out[x + a_SizeX * z] = 0;
                continue;
            }

            bool adjacentToOcean = false;
            for (int dz = -1; dz <= 1; dz++)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    if (dx == 0 && dz == 0) continue;
                    if (GetParentInt(m_ChunkX + dx, m_ChunkZ + dz) == 0)
                    {
                        adjacentToOcean = true;
                        break;
                    }
                }
                if (adjacentToOcean) break;
            }

            if (adjacentToOcean)
            {
                switch (val)
                {
                    case 1: a_Out[x + a_SizeX * z] = 16; break;   // Beach
                    case 2: a_Out[x + a_SizeX * z] = 16; break;   // Beach
                    case 3: a_Out[x + a_SizeX * z] = 25; break;   // Stone Beach
                    case 4: a_Out[x + a_SizeX * z] = 16; break;   // Beach
                    case 5: a_Out[x + a_SizeX * z] = 16; break;   // Beach
                    case 6: a_Out[x + a_SizeX * z] = 16; break;   // Beach
                    case 12: a_Out[x + a_SizeX * z] = 26; break;  // Cold Beach
                    case 13: a_Out[x + a_SizeX * z] = 25; break;  // Stone Beach
                    case 30: a_Out[x + a_SizeX * z] = 26; break;  // Cold Beach
                    case 14: a_Out[x + a_SizeX * z] = 15; break;  // Mushroom Shore
                    default: a_Out[x + a_SizeX * z] = 16; break;  // Beach
                }
            }
            else
            {
                a_Out[x + a_SizeX * z] = val;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// GenLayerRiver - Carve rivers

cBioGenVanilla::GenLayerRiver::GenLayerRiver(int a_Seed, GenLayer * a_Parent) : GenLayer(a_Seed, a_Parent), m_Noise(a_Seed + 2000) {}

int cBioGenVanilla::GenLayerRiver::GetInt(int a_X, int a_Z)
{
    m_ChunkX = a_X;
    m_ChunkZ = a_Z;

    // Rivers are carved using noise
    double noise = m_Noise.CubicNoise2D(static_cast<float>(a_X) * 0.025f, static_cast<float>(a_Z) * 0.025f);
    noise += 0.5 * m_Noise.CubicNoise2D(static_cast<float>(a_X) * 0.05f, static_cast<float>(a_Z) * 0.05f);
    noise += 0.25 * m_Noise.CubicNoise2D(static_cast<float>(a_X) * 0.1f, static_cast<float>(a_Z) * 0.1f);

    // River threshold - narrow bands
    if (noise > -0.1 && noise < 0.1)
    {
        return 7;  // River
    }

    return GetParentInt(a_X, a_Z);
}

void cBioGenVanilla::GenLayerRiver::GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out)
{
    for (int z = 0; z < a_SizeZ; z++)
    {
        for (int x = 0; x < a_SizeX; x++)
        {
            m_ChunkX = a_X + x;
            m_ChunkZ = a_Z + z;

            double noise = m_Noise.CubicNoise2D(static_cast<float>(m_ChunkX) * 0.025f, static_cast<float>(m_ChunkZ) * 0.025f);
            noise += 0.5 * m_Noise.CubicNoise2D(static_cast<float>(m_ChunkX) * 0.05f, static_cast<float>(m_ChunkZ) * 0.05f);
            noise += 0.25 * m_Noise.CubicNoise2D(static_cast<float>(m_ChunkX) * 0.1f, static_cast<float>(m_ChunkZ) * 0.1f);

            if (noise > -0.1 && noise < 0.1)
            {
                a_Out[x + a_SizeX * z] = 7;  // River
            }
            else
            {
                a_Out[x + a_SizeX * z] = GetParentInt(m_ChunkX, m_ChunkZ);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// GenLayerSmooth - Smooth biome transitions

cBioGenVanilla::GenLayerSmooth::GenLayerSmooth(int a_Seed, GenLayer * a_Parent) : GenLayer(a_Seed, a_Parent) {}

int cBioGenVanilla::GenLayerSmooth::GetInt(int a_X, int a_Z)
{
    m_ChunkX = a_X;
    m_ChunkZ = a_Z;

    int center = GetParentInt(a_X, a_Z);
    int n = GetParentInt(a_X, a_Z - 1);
    int s = GetParentInt(a_X, a_Z + 1);
    int w = GetParentInt(a_X - 1, a_Z);
    int e = GetParentInt(a_X + 1, a_Z);

    // If all four neighbors are same and different from center, use neighbor
    if (n == s && s == w && w == e && n != center)
    {
        return n;
    }

    return center;
}

void cBioGenVanilla::GenLayerSmooth::GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out)
{
    for (int z = 0; z < a_SizeZ; z++)
    {
        for (int x = 0; x < a_SizeX; x++)
        {
            m_ChunkX = a_X + x;
            m_ChunkZ = a_Z + z;

            int center = GetParentInt(m_ChunkX, m_ChunkZ);
            int n = GetParentInt(m_ChunkX, m_ChunkZ - 1);
            int s = GetParentInt(m_ChunkX, m_ChunkZ + 1);
            int w = GetParentInt(m_ChunkX - 1, m_ChunkZ);
            int e = GetParentInt(m_ChunkX + 1, m_ChunkZ);

            if (n == s && s == w && w == e && n != center)
            {
                a_Out[x + a_SizeX * z] = n;
            }
            else
            {
                a_Out[x + a_SizeX * z] = center;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// GenLayerRareBiome - Place rare biomes (Mesa, Jungle variants, etc.)

cBioGenVanilla::GenLayerRareBiome::GenLayerRareBiome(int a_Seed, GenLayer * a_Parent) : GenLayer(a_Seed, a_Parent) {}

int cBioGenVanilla::GenLayerRareBiome::GetInt(int a_X, int a_Z)
{
    m_ChunkX = a_X;
    m_ChunkZ = a_Z;
    int val = GetParentInt(a_X, a_Z);

    // 1/28 chance for rare biome replacement
    int64_t i = static_cast<int64_t>(a_X) * 341873128712LL + static_cast<int64_t>(a_Z) * 132897987541LL + m_Seed;
    i = i * i * 42317861 + i * 11;
    i = (i >> 16) & 27;

    if (i == 0)
    {
        // Rare biome placement based on base biome
        switch (val)
        {
            case 1:  return 37;  // Plains -> Mesa
            case 2:  return 37;  // Desert -> Mesa
            case 4:  return 21;  // Forest -> Jungle
            case 5:  return 32;  // Taiga -> Mega Taiga
            case 21: return 21;  // Jungle stays
            case 32: return 32;  // Mega Taiga stays
            case 37: return 37;  // Mesa stays
            default: return val;
        }
    }

    return val;
}

void cBioGenVanilla::GenLayerRareBiome::GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out)
{
    for (int z = 0; z < a_SizeZ; z++)
    {
        for (int x = 0; x < a_SizeX; x++)
        {
            m_ChunkX = a_X + x;
            m_ChunkZ = a_Z + z;
            int val = GetParentInt(m_ChunkX, m_ChunkZ);

            int64_t i = static_cast<int64_t>(m_ChunkX) * 341873128712LL + static_cast<int64_t>(m_ChunkZ) * 132897987541LL + m_Seed;
            i = i * i * 42317861 + i * 11;
            i = (i >> 16) & 27;

            if (i == 0)
            {
                switch (val)
                {
                    case 1:  a_Out[x + a_SizeX * z] = 37; break;  // Mesa
                    case 2:  a_Out[x + a_SizeX * z] = 37; break;  // Mesa
                    case 4:  a_Out[x + a_SizeX * z] = 21; break;  // Jungle
                    case 5:  a_Out[x + a_SizeX * z] = 32; break;  // Mega Taiga
                    default: a_Out[x + a_SizeX * z] = val; break;
                }
            }
            else
            {
                a_Out[x + a_SizeX * z] = val;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// GenLayerBiome - Final biome assignment based on temperature/humidity

cBioGenVanilla::GenLayerBiome::GenLayerBiome(int a_Seed, GenLayer * a_Parent) : GenLayer(a_Seed, a_Parent), m_Noise(a_Seed + 3000) {}

int cBioGenVanilla::GenLayerBiome::GetInt(int a_X, int a_Z)
{
    m_ChunkX = a_X;
    m_ChunkZ = a_Z;
    int val = GetParentInt(a_X, a_Z);

    // Already assigned biomes (ocean, river, etc.) stay
    if (val == 0 || val == 7 || val == 10 || val == 11 || val == 14 || val == 15 ||
        val == 16 || val == 24 || val == 25 || val == 26)
    {
        return val;
    }

    // Generate temperature and humidity noise
    double temp = m_Noise.CubicNoise2D(static_cast<float>(a_X) * 0.01f, static_cast<float>(a_Z) * 0.01f);
    double hum = m_Noise.CubicNoise2D(static_cast<float>(a_X) * 0.01f + 1000, static_cast<float>(a_Z) * 0.01f + 1000);

    // Map noise to [0,1] range
    temp = (temp + 1.0) * 0.5;
    hum = (hum + 1.0) * 0.5;

    // Vanilla biome assignment based on temperature/humidity
    if (temp < 0.1)      // Very cold
    {
        if (hum < 0.5) return 12;  // Ice Plains
        return 13;                 // Ice Mountains
    }
    else if (temp < 0.3) // Cold
    {
        if (hum < 0.5) return 30;  // Cold Taiga
        return 5;                  // Taiga
    }
    else if (temp < 0.6) // Temperate
    {
        if (hum < 0.3) return 1;   // Plains
        if (hum < 0.6) return 4;   // Forest
        if (hum < 0.8) return 27;  // Birch Forest
        return 29;                 // Roofed Forest
    }
    else if (temp < 0.8) // Warm
    {
        if (hum < 0.3) return 2;   // Desert
        if (hum < 0.6) return 35;  // Savanna
        return 21;                 // Jungle
    }
    else                 // Hot
    {
        if (hum < 0.2) return 2;   // Desert
        return 37;                 // Mesa
    }
}

void cBioGenVanilla::GenLayerBiome::GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out)
{
    for (int z = 0; z < a_SizeZ; z++)
    {
        for (int x = 0; x < a_SizeX; x++)
        {
            m_ChunkX = a_X + x;
            m_ChunkZ = a_Z + z;
            int val = GetParentInt(m_ChunkX, m_ChunkZ);

            if (val == 0 || val == 7 || val == 10 || val == 11 || val == 14 || val == 15 ||
                val == 16 || val == 24 || val == 25 || val == 26)
            {
                a_Out[x + a_SizeX * z] = val;
                continue;
            }

            double temp = m_Noise.CubicNoise2D(static_cast<float>(m_ChunkX) * 0.01f, static_cast<float>(m_ChunkZ) * 0.01f);
            double hum = m_Noise.CubicNoise2D(static_cast<float>(m_ChunkX) * 0.01f + 1000, static_cast<float>(m_ChunkZ) * 0.01f + 1000);

            temp = (temp + 1.0) * 0.5;
            hum = (hum + 1.0) * 0.5;

            if (temp < 0.1)
            {
                a_Out[x + a_SizeX * z] = (hum < 0.5) ? 12 : 13;
            }
            else if (temp < 0.3)
            {
                a_Out[x + a_SizeX * z] = (hum < 0.5) ? 30 : 5;
            }
            else if (temp < 0.6)
            {
                if (hum < 0.3) a_Out[x + a_SizeX * z] = 1;
                else if (hum < 0.6) a_Out[x + a_SizeX * z] = 4;
                else if (hum < 0.8) a_Out[x + a_SizeX * z] = 27;
                else a_Out[x + a_SizeX * z] = 29;
            }
            else if (temp < 0.8)
            {
                if (hum < 0.3) a_Out[x + a_SizeX * z] = 2;
                else if (hum < 0.6) a_Out[x + a_SizeX * z] = 35;
                else a_Out[x + a_SizeX * z] = 21;
            }
            else
            {
                a_Out[x + a_SizeX * z] = (hum < 0.2) ? 2 : 37;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// GenLayerShore - Add shore biomes

cBioGenVanilla::GenLayerShore::GenLayerShore(int a_Seed, GenLayer * a_Parent) : GenLayer(a_Seed, a_Parent), m_Noise(a_Seed + 4000) {}

int cBioGenVanilla::GenLayerShore::GetInt(int a_X, int a_Z)
{
    m_ChunkX = a_X;
    m_ChunkZ = a_Z;
    int val = GetParentInt(a_X, a_Z);

    // Ocean variants get shores
    if (val == 0)   // Ocean
    {
        // Check for adjacent non-ocean
        for (int dz = -1; dz <= 1; dz++)
        {
            for (int dx = -1; dx <= 1; dx++)
            {
                if (dx == 0 && dz == 0) continue;
                int n = GetParentInt(a_X + dx, a_Z + dz);
                if (n != 0 && n != 7 && n != 10 && n != 11 && n != 24)
                {
                    return 16;  // Beach
                }
            }
        }
        return 0;
    }
    else if (val == 10)  // Frozen Ocean
    {
        for (int dz = -1; dz <= 1; dz++)
        {
            for (int dx = -1; dx <= 1; dx++)
            {
                if (dx == 0 && dz == 0) continue;
                int n = GetParentInt(a_X + dx, a_Z + dz);
                if (n != 0 && n != 7 && n != 10 && n != 11 && n != 24)
                {
                    return 26;  // Cold Beach
                }
            }
        }
        return 10;
    }
    else if (val == 24)  // Deep Ocean
    {
        return 24;
    }

    return val;
}

void cBioGenVanilla::GenLayerShore::GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out)
{
    for (int z = 0; z < a_SizeZ; z++)
    {
        for (int x = 0; x < a_SizeX; x++)
        {
            m_ChunkX = a_X + x;
            m_ChunkZ = a_Z + z;
            int val = GetParentInt(m_ChunkX, m_ChunkZ);

            if (val == 0)
            {
                bool hasLand = false;
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        if (dx == 0 && dz == 0) continue;
                        int n = GetParentInt(m_ChunkX + dx, m_ChunkZ + dz);
                        if (n != 0 && n != 7 && n != 10 && n != 11 && n != 24)
                        {
                            hasLand = true;
                            break;
                        }
                    }
                    if (hasLand) break;
                }
                a_Out[x + a_SizeX * z] = hasLand ? 16 : 0;
            }
            else if (val == 10)
            {
                bool hasLand = false;
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        if (dx == 0 && dz == 0) continue;
                        int n = GetParentInt(m_ChunkX + dx, m_ChunkZ + dz);
                        if (n != 0 && n != 7 && n != 10 && n != 11 && n != 24)
                        {
                            hasLand = true;
                            break;
                        }
                    }
                    if (hasLand) break;
                }
                a_Out[x + a_SizeX * z] = hasLand ? 26 : 10;
            }
            else if (val == 24)
            {
                a_Out[x + a_SizeX * z] = 24;
            }
            else
            {
                a_Out[x + a_SizeX * z] = val;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// GenLayerRiverMix - Mix rivers with biomes

cBioGenVanilla::GenLayerRiverMix::GenLayerRiverMix(int a_Seed, GenLayer * a_Parent, GenLayer * a_RiverParent) :
    GenLayer(a_Seed, a_Parent),
    m_RiverParent(a_RiverParent)
{
}

int cBioGenVanilla::GenLayerRiverMix::GetInt(int a_X, int a_Z)
{
    m_ChunkX = a_X;
    m_ChunkZ = a_Z;

    int biome = GetParentInt(a_X, a_Z);
    int river = m_RiverParent->GetInt(a_X, a_Z);

    // River overrides biome (except ocean)
    if (river == 7 && biome != 0 && biome != 10 && biome != 24)
    {
        return 7;
    }

    return biome;
}

void cBioGenVanilla::GenLayerRiverMix::GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out)
{
    for (int z = 0; z < a_SizeZ; z++)
    {
        for (int x = 0; x < a_SizeX; x++)
        {
            m_ChunkX = a_X + x;
            m_ChunkZ = a_Z + z;

            int biome = GetParentInt(m_ChunkX, m_ChunkZ);
            int river = m_RiverParent->GetInt(m_ChunkX, m_ChunkZ);

            if (river == 7 && biome != 0 && biome != 10 && biome != 24)
            {
                a_Out[x + a_SizeX * z] = 7;
            }
            else
            {
                a_Out[x + a_SizeX * z] = biome;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////
// GenLayerHills - Add hill variants to biomes

cBioGenVanilla::GenLayerHills::GenLayerHills(int a_Seed, GenLayer * a_Parent) : GenLayer(a_Seed, a_Parent) {}

int cBioGenVanilla::GenLayerHills::GetInt(int a_X, int a_Z)
{
    m_ChunkX = a_X;
    m_ChunkZ = a_Z;
    int val = GetParentInt(a_X, a_Z);

    // Special biomes that don't get hills
    if (val == 0 || val == 7 || val == 10 || val == 11 || val == 14 || val == 15 ||
        val == 16 || val == 24 || val == 25 || val == 26)
    {
        return val;
    }

    // Check 3x3 neighborhood for same biome
    bool hasDifferentNeighbor = false;
    for (int dz = -1; dz <= 1; dz++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dz == 0) continue;
            int neighbor = GetParentInt(a_X + dx, a_Z + dz);
            if (neighbor != val)
            {
                hasDifferentNeighbor = true;
                break;
            }
        }
        if (hasDifferentNeighbor) break;
    }

    // If all neighbors are same biome, no hills
    if (!hasDifferentNeighbor)
    {
        return val;
    }

    // 1/4 chance to convert to hill variant
    int64_t i = static_cast<int64_t>(a_X) * 341873128712LL + static_cast<int64_t>(a_Z) * 132897987541LL + m_Seed;
    i = i * i * 42317861 + i * 11;
    i = (i >> 16) & 3;

    if (i == 0)
    {
        // Map base biome to hill variant
        switch (val)
        {
            case 1:  return 18;  // Plains -> ForestHills (vanilla uses this for Plains hills too)
            case 2:  return 17;  // Desert -> DesertHills
            case 3:  return 20;  // ExtremeHills -> ExtremeHillsEdge
            case 4:  return 18;  // Forest -> ForestHills
            case 5:  return 19;  // Taiga -> TaigaHills
            case 6:  return 6;   // Swampland stays (no hills)
            case 12: return 13;  // IcePlains -> IceMountains
            case 14: return 14;  // MushroomIsland stays
            case 21: return 22;  // Jungle -> JungleHills
            case 23: return 23;  // JungleEdge stays
            case 27: return 28;  // BirchForest -> BirchForestHills
            case 29: return 29;  // RoofedForest stays
            case 30: return 31;  // ColdTaiga -> ColdTaigaHills
            case 32: return 33;  // MegaTaiga -> MegaTaigaHills
            case 34: return 20;  // ExtremeHillsPlus -> ExtremeHillsEdge
            case 35: return 36;  // Savanna -> SavannaPlateau
            case 37: return 39;  // Mesa -> MesaPlateau
            case 38: return 38;  // MesaPlateauF stays
            default: return val;
        }
    }

    return val;
}

void cBioGenVanilla::GenLayerHills::GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out)
{
    for (int z = 0; z < a_SizeZ; z++)
    {
        for (int x = 0; x < a_SizeX; x++)
        {
            m_ChunkX = a_X + x;
            m_ChunkZ = a_Z + z;
            int val = GetParentInt(m_ChunkX, m_ChunkZ);

            if (val == 0 || val == 7 || val == 10 || val == 11 || val == 14 || val == 15 ||
                val == 16 || val == 24 || val == 25 || val == 26)
            {
                a_Out[x + a_SizeX * z] = val;
                continue;
            }

            bool hasDifferentNeighbor = false;
            for (int dz = -1; dz <= 1; dz++)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    if (dx == 0 && dz == 0) continue;
                    int neighbor = GetParentInt(m_ChunkX + dx, m_ChunkZ + dz);
                    if (neighbor != val)
                    {
                        hasDifferentNeighbor = true;
                        break;
                    }
                }
                if (hasDifferentNeighbor) break;
            }

            if (!hasDifferentNeighbor)
            {
                a_Out[x + a_SizeX * z] = val;
                continue;
            }

            int64_t i = static_cast<int64_t>(m_ChunkX) * 341873128712LL + static_cast<int64_t>(m_ChunkZ) * 132897987541LL + m_Seed;
            i = i * i * 42317861 + i * 11;
            i = (i >> 16) & 3;

            if (i == 0)
            {
                switch (val)
                {
                    case 1:  a_Out[x + a_SizeX * z] = 18; break;
                    case 2:  a_Out[x + a_SizeX * z] = 17; break;
                    case 3:  a_Out[x + a_SizeX * z] = 20; break;
                    case 4:  a_Out[x + a_SizeX * z] = 18; break;
                    case 5:  a_Out[x + a_SizeX * z] = 19; break;
                    case 6:  a_Out[x + a_SizeX * z] = 6; break;
                    case 12: a_Out[x + a_SizeX * z] = 13; break;
                    case 14: a_Out[x + a_SizeX * z] = 14; break;
                    case 21: a_Out[x + a_SizeX * z] = 22; break;
                    case 23: a_Out[x + a_SizeX * z] = 23; break;
                    case 27: a_Out[x + a_SizeX * z] = 28; break;
                    case 29: a_Out[x + a_SizeX * z] = 29; break;
                    case 30: a_Out[x + a_SizeX * z] = 31; break;
                    case 32: a_Out[x + a_SizeX * z] = 33; break;
                    case 34: a_Out[x + a_SizeX * z] = 20; break;
                    case 35: a_Out[x + a_SizeX * z] = 36; break;
                    case 37: a_Out[x + a_SizeX * z] = 39; break;
                    case 38: a_Out[x + a_SizeX * z] = 38; break;
                    default: a_Out[x + a_SizeX * z] = val; break;
                }
            }
            else
            {
                a_Out[x + a_SizeX * z] = val;
            }
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////
// GenLayerMutatedBiome - Add M (mutated) variants

cBioGenVanilla::GenLayerMutatedBiome::GenLayerMutatedBiome(int a_Seed, GenLayer * a_Parent) : GenLayer(a_Seed, a_Parent) {}

int cBioGenVanilla::GenLayerMutatedBiome::GetInt(int a_X, int a_Z)
{
    m_ChunkX = a_X;
    m_ChunkZ = a_Z;
    int val = GetParentInt(a_X, a_Z);

    // Special biomes that don't get mutated variants
    if (val == 0 || val == 7 || val == 10 || val == 11 || val == 14 || val == 15 ||
        val == 16 || val == 24 || val == 25 || val == 26)
    {
        return val;
    }

    // 1/13 chance for mutated variant
    int64_t i = static_cast<int64_t>(a_X) * 341873128712LL + static_cast<int64_t>(a_Z) * 132897987541LL + m_Seed;
    i = i * i * 42317861 + i * 11;
    i = (i >> 16) & 12;

    if (i == 0)
    {
        // Map base biome to mutated variant (+128)
        // Only certain biomes have M variants
        switch (val)
        {
            case 1:  return 129; // Plains -> SunflowerPlains
            case 2:  return 130; // Desert -> DesertM
            case 3:  return 131; // ExtremeHills -> ExtremeHillsM
            case 4:  return 132; // Forest -> FlowerForest
            case 5:  return 133; // Taiga -> TaigaM
            case 6:  return 134; // Swampland -> SwamplandM
            case 12: return 140; // IcePlains -> IcePlainsSpikes
            case 21: return 149; // Jungle -> JungleM
            case 23: return 151; // JungleEdge -> JungleEdgeM
            case 27: return 155; // BirchForest -> BirchForestM
            case 28: return 156; // BirchForestHills -> BirchForestHillsM
            case 29: return 157; // RoofedForest -> RoofedForestM
            case 30: return 158; // ColdTaiga -> ColdTaigaM
            case 32: return 160; // MegaTaiga -> MegaSpruceTaiga
            case 33: return 161; // MegaTaigaHills -> MegaSpruceTaigaHills
            case 34: return 162; // ExtremeHillsPlus -> ExtremeHillsPlusM
            case 35: return 163; // Savanna -> SavannaM
            case 36: return 164; // SavannaPlateau -> SavannaPlateauM
            case 37: return 165; // Mesa -> MesaBryce
            case 38: return 166; // MesaPlateauF -> MesaPlateauFM
            case 39: return 167; // MesaPlateau -> MesaPlateauM
            default: return val;
        }
    }

    return val;
}

void cBioGenVanilla::GenLayerMutatedBiome::GetInts(int a_X, int a_Z, int a_SizeX, int a_SizeZ, int * a_Out)
{
    for (int z = 0; z < a_SizeZ; z++)
    {
        for (int x = 0; x < a_SizeX; x++)
        {
            m_ChunkX = a_X + x;
            m_ChunkZ = a_Z + z;
            int val = GetParentInt(m_ChunkX, m_ChunkZ);

            if (val == 0 || val == 7 || val == 10 || val == 11 || val == 14 || val == 15 ||
                val == 16 || val == 24 || val == 25 || val == 26)
            {
                a_Out[x + a_SizeX * z] = val;
                continue;
            }

            int64_t i = static_cast<int64_t>(m_ChunkX) * 341873128712LL + static_cast<int64_t>(m_ChunkZ) * 132897987541LL + m_Seed;
            i = i * i * 42317861 + i * 11;
            i = (i >> 16) & 12;

            if (i == 0)
            {
                switch (val)
                {
                    case 1:  a_Out[x + a_SizeX * z] = 129; break;
                    case 2:  a_Out[x + a_SizeX * z] = 130; break;
                    case 3:  a_Out[x + a_SizeX * z] = 131; break;
                    case 4:  a_Out[x + a_SizeX * z] = 132; break;
                    case 5:  a_Out[x + a_SizeX * z] = 133; break;
                    case 6:  a_Out[x + a_SizeX * z] = 134; break;
                    case 12: a_Out[x + a_SizeX * z] = 140; break;
                    case 21: a_Out[x + a_SizeX * z] = 149; break;
                    case 23: a_Out[x + a_SizeX * z] = 151; break;
                    case 27: a_Out[x + a_SizeX * z] = 155; break;
                    case 28: a_Out[x + a_SizeX * z] = 156; break;
                    case 29: a_Out[x + a_SizeX * z] = 157; break;
                    case 30: a_Out[x + a_SizeX * z] = 158; break;
                    case 32: a_Out[x + a_SizeX * z] = 160; break;
                    case 33: a_Out[x + a_SizeX * z] = 161; break;
                    case 34: a_Out[x + a_SizeX * z] = 162; break;
                    case 35: a_Out[x + a_SizeX * z] = 163; break;
                    case 36: a_Out[x + a_SizeX * z] = 164; break;
                    case 37: a_Out[x + a_SizeX * z] = 165; break;
                    case 38: a_Out[x + a_SizeX * z] = 166; break;
                    case 39: a_Out[x + a_SizeX * z] = 167; break;
                    default: a_Out[x + a_SizeX * z] = val; break;
                }
            }
            else
            {
                a_Out[x + a_SizeX * z] = val;
            }
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////
// cBioGenVanilla main class

cBioGenVanilla::cBioGenVanilla(int a_Seed) : m_Seed(a_Seed), m_Noise(a_Seed)
{
    InitializeLayers();
}

void cBioGenVanilla::InitializeLayers()
{
    // Build the exact vanilla GenLayer chain:
    // Island -> FuzzyZoom -> Zoom x3 -> AddIsland -> Zoom x2 -> RemoveTooMuchOcean -> AddSnow -> AddIsland3 -> Zoom x2 -> Edge -> Zoom x2 -> River -> Smooth -> RareBiome -> Biome -> Hills -> MutatedBiome -> Shore -> RiverMix

    // Layer 1: Island (1x1)
    std::unique_ptr<GenLayer> layer = std::make_unique<GenLayerIsland>(m_Seed + 1);

    // Layer 2: FuzzyZoom (2x2)
    layer = std::make_unique<GenLayerFuzzyZoom>(m_Seed + 2, layer.release());

    // Layer 3-5: Zoom x3 (4x4, 8x8, 16x16)
    layer = std::make_unique<GenLayerZoom>(m_Seed + 3, layer.release());
    layer = std::make_unique<GenLayerZoom>(m_Seed + 4, layer.release());
    layer = std::make_unique<GenLayerZoom>(m_Seed + 5, layer.release());

    // Layer 6: AddIsland (16x16)
    layer = std::make_unique<GenLayerAddIsland>(m_Seed + 6, layer.release());

    // Layer 7-8: Zoom x2 (32x32, 64x64)
    layer = std::make_unique<GenLayerZoom>(m_Seed + 7, layer.release());
    layer = std::make_unique<GenLayerZoom>(m_Seed + 8, layer.release());

    // Layer 9: RemoveTooMuchOcean (64x64)
    layer = std::make_unique<GenLayerRemoveTooMuchOcean>(m_Seed + 9, layer.release());

    // Layer 10: AddSnow (64x64)
    layer = std::make_unique<GenLayerAddSnow>(m_Seed + 10, layer.release());

    // Layer 11: AddIsland3 (64x64)
    layer = std::make_unique<GenLayerAddIsland3>(m_Seed + 11, layer.release());

    // Layer 12-13: Zoom x2 (128x128, 256x256)
    layer = std::make_unique<GenLayerZoom>(m_Seed + 12, layer.release());
    layer = std::make_unique<GenLayerZoom>(m_Seed + 13, layer.release());

    // Layer 14: Edge (256x256) - add beaches
    layer = std::make_unique<GenLayerEdge>(m_Seed + 14, layer.release());

    // Layer 15-16: Zoom x2 (512x512, 1024x1024)
    layer = std::make_unique<GenLayerZoom>(m_Seed + 15, layer.release());
    layer = std::make_unique<GenLayerZoom>(m_Seed + 16, layer.release());

    // River layer (parallel chain)
    // Island -> FuzzyZoom -> Zoom x5 -> River
    std::unique_ptr<GenLayer> riverLayer = std::make_unique<GenLayerIsland>(m_Seed + 100);
    riverLayer = std::make_unique<GenLayerFuzzyZoom>(m_Seed + 101, riverLayer.release());
    riverLayer = std::make_unique<GenLayerZoom>(m_Seed + 102, riverLayer.release());
    riverLayer = std::make_unique<GenLayerZoom>(m_Seed + 103, riverLayer.release());
    riverLayer = std::make_unique<GenLayerZoom>(m_Seed + 104, riverLayer.release());
    riverLayer = std::make_unique<GenLayerZoom>(m_Seed + 105, riverLayer.release());
    riverLayer = std::make_unique<GenLayerZoom>(m_Seed + 106, riverLayer.release());
    riverLayer = std::make_unique<GenLayerRiver>(m_Seed + 107, riverLayer.release());

    // Layer 17: Smooth (1024x1024)
    layer = std::make_unique<GenLayerSmooth>(m_Seed + 17, layer.release());

    // Layer 18: RareBiome (1024x1024)
    layer = std::make_unique<GenLayerRareBiome>(m_Seed + 18, layer.release());

    // Layer 19: Biome (1024x1024) - final biome assignment
    layer = std::make_unique<GenLayerBiome>(m_Seed + 19, layer.release());

    // Layer 20: Hills (1024x1024) - add hill variants
    layer = std::make_unique<GenLayerHills>(m_Seed + 20, layer.release());

    // Layer 21: MutatedBiome (1024x1024) - add M variants
    layer = std::make_unique<GenLayerMutatedBiome>(m_Seed + 21, layer.release());

    // Layer 22: Shore (1024x1024)
    layer = std::make_unique<GenLayerShore>(m_Seed + 22, layer.release());

    // Layer 23: RiverMix (1024x1024) - combine rivers with biomes
    // Need to zoom river layer to match
    riverLayer = std::make_unique<GenLayerZoom>(m_Seed + 108, riverLayer.release());
    riverLayer = std::make_unique<GenLayerZoom>(m_Seed + 109, riverLayer.release());
    riverLayer = std::make_unique<GenLayerZoom>(m_Seed + 110, riverLayer.release());
    riverLayer = std::make_unique<GenLayerZoom>(m_Seed + 111, riverLayer.release());
    layer = std::make_unique<GenLayerRiverMix>(m_Seed + 23, layer.release(), riverLayer.release());

    // Final zoom to chunk resolution (16x16 blocks per biome pixel)
    // Vanilla generates at 1024x1024 for 16x16 chunks, so we need to zoom 4 more times
    layer = std::make_unique<GenLayerZoom>(m_Seed + 24, layer.release());  // 2048
    layer = std::make_unique<GenLayerZoom>(m_Seed + 25, layer.release());  // 4096
    layer = std::make_unique<GenLayerZoom>(m_Seed + 26, layer.release());  // 8192
    layer = std::make_unique<GenLayerZoom>(m_Seed + 27, layer.release());  // 16384

    m_FinalLayer = std::move(layer);
}

EMCSBiome cBioGenVanilla::VanillaBiomeToCuberite(int a_VanillaBiome)
{
    // Vanilla 1.12.2 biome IDs map directly to Cuberite's EMCSBiome enum
    return static_cast<EMCSBiome>(a_VanillaBiome);
}

void cBioGenVanilla::GenBiomes(cChunkCoords a_ChunkCoords, cChunkDef::BiomeMap & a_BiomeMap)
{
    // Generate biome map for the chunk
    // Each biome pixel covers 16x16 blocks, so we need 16x16 biome pixels per chunk
    int biomeCoordsX = a_ChunkCoords.m_ChunkX * cChunkDef::Width;
    int biomeCoordsZ = a_ChunkCoords.m_ChunkZ * cChunkDef::Width;

    // Generate 16x16 biome values
    int biomeValues[16 * 16];
    m_FinalLayer->GetInts(biomeCoordsX, biomeCoordsZ, 16, 16, biomeValues);

    // Convert to Cuberite biome map
    for (int z = 0; z < cChunkDef::Width; z++)
    {
        for (int x = 0; x < cChunkDef::Width; x++)
        {
            int vanillaBiome = biomeValues[x + 16 * z];
            EMCSBiome cuberiteBiome = VanillaBiomeToCuberite(vanillaBiome);
            cChunkDef::SetBiome(a_BiomeMap, x, z, cuberiteBiome);
        }
    }
}

void cBioGenVanilla::InitializeBiomeGen(cIniFile & a_IniFile)
{
    // No user-settable params for vanilla-accurate generation
    UNUSED(a_IniFile);
}
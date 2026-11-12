#pragma once

#include <cstdint>
#include <vector>

#include "../utils/Biome.hh"
#include "IslandMask.hh"
#include "SpatialUnits.hh"
#include "SpawnTable.hh"

namespace NocInDan
{

    struct WorldGenConfig
    {
        static constexpr int kMaxMapSize               = 256;
        static constexpr int kSpawnBudgetReferenceArea = 64 * 64;

        uint64_t m_seed = 0;

        // Playable grid size in cells, in [1, kMaxMapSize].
        int        m_width  = 128;
        int        m_height = 128;
        WorldUnits m_cellSize{ 8.0f };

        IslandMaskParams m_island{};

        // Region centers to plant. Raised to at least one per land biome in m_biomes.
        int m_regionCount = 8;

        Cells m_playerClearRadius{ 0 };

        // Authored against kSpawnBudgetReferenceArea; scaled by land area at generate time.
        int m_maxResources = 6000;
        int m_maxValuables = 200;
        int m_maxThreats   = 200;

        std::vector<BiomeDef> m_biomes;
    };

    WorldGenConfig makeDefaultWorldGenConfig( uint64_t p_seed = 0 );

}  // namespace NocInDan

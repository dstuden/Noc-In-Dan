#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../utils/Biome.hh"
#include "GroundTileId.hh"
#include "SpatialUnits.hh"

namespace NocInDan
{

    enum class SpawnPass : uint8_t
    {
        SetPiece = 0,
        Resource = 1,
        Valuable = 2,
        Threat   = 3,
        Count
    };

    struct SpawnEntry
    {
        std::string m_prefabId;
        SpawnPass   m_pass = SpawnPass::Resource;

        // Average spawns per eligible tile across the biome; positions are free, not per tile.
        float m_density = 0.0f;
        // noise clumping (0 = uniform); cutoff ~ fraction left as clearings
        float m_clumpFreq   = 0.0f;
        float m_clumpCutoff = 0.5f;

        WorldUnits m_minSpacing{ 2.0f };

        bool m_avoidPlayerSpawn = true;
        bool m_requireInterior  = true;
    };

    // One weighted ground tile in a biome's turf mix.
    struct GroundMixEntry
    {
        GroundTileId m_tile   = GroundTileId::Grass;
        float        m_weight = 1.0f;
    };

    struct BiomeDef
    {
        BiomeId                 m_id               = BiomeId::None;
        float                   m_regionWeight     = 1.0f;
        bool                    m_allowPlayerSpawn = false;
        std::vector<SpawnEntry> m_spawns;

        // Turf mix painted with value noise; required (positive weights) for land biomes.
        std::vector<GroundMixEntry> m_groundMix;
        float                       m_groundNoiseFreq = 0.12f;
    };

}  // namespace NocInDan

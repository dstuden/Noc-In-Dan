#include "WorldGenConfig.hh"

namespace NocInDan
{

    WorldGenConfig makeDefaultWorldGenConfig( uint64_t p_seed )
    {
        WorldGenConfig cfg;
        cfg.m_seed        = p_seed;
        cfg.m_width       = 64;
        cfg.m_height      = 64;
        cfg.m_regionCount = 8;

        cfg.m_biomes = {
                { .m_id               = BiomeId::Plains,
                  .m_regionWeight     = 1.0f,
                  .m_allowPlayerSpawn = true,
                  .m_spawns =
                          {
                                  { .m_prefabId    = "grass",
                                    .m_density     = 1.5f,
                                    .m_clumpFreq   = 0.25f,
                                    .m_clumpCutoff = 0.45f,
                                    .m_minSpacing  = WorldUnits{ 1.5f } },

                                  { .m_prefabId = "tree", .m_density = 0.3f, .m_clumpFreq = 0.20f, .m_clumpCutoff = 0.65f, .m_minSpacing = WorldUnits{ 2.5f } },
                          },
                  .m_groundMix = { { GroundTileId::Grass, 0.8f }, { GroundTileId::Rocky, 0.2f } } },
                { .m_id               = BiomeId::Forest,
                  .m_regionWeight     = 1.4f,
                  .m_allowPlayerSpawn = true,
                  .m_spawns =
                          {
                                  { .m_prefabId    = "grass",
                                    .m_density     = 0.4f,
                                    .m_clumpFreq   = 0.30f,
                                    .m_clumpCutoff = 0.50f,
                                    .m_minSpacing  = WorldUnits{ 1.5f } },

                                  { .m_prefabId = "tree", .m_density = 2.5f, .m_clumpFreq = 0.15f, .m_clumpCutoff = 0.35f, .m_minSpacing = WorldUnits{ 2.5f } },
                          },
                  .m_groundMix = { { GroundTileId::Forest, 0.75f }, { GroundTileId::Grass, 0.25f } } },
                { .m_id               = BiomeId::Swamp,
                  .m_regionWeight     = 0.7f,
                  .m_allowPlayerSpawn = false,
                  .m_spawns           = {},
                  .m_groundMix        = { { GroundTileId::Marsh, 1.0f } } },
                { .m_id               = BiomeId::Desert,
                  .m_regionWeight     = 0.8f,
                  .m_allowPlayerSpawn = false,
                  .m_spawns           = {},
                  .m_groundMix        = { { GroundTileId::Sand, 0.85f }, { GroundTileId::Rocky, 0.15f } } },
                { .m_id               = BiomeId::Tundra,
                  .m_regionWeight     = 0.55f,
                  .m_allowPlayerSpawn = false,
                  .m_spawns           = {},
                  .m_groundMix        = { { GroundTileId::Snow, 0.8f }, { GroundTileId::Rocky, 0.2f } } },
        };

        return cfg;
    }

}  // namespace NocInDan

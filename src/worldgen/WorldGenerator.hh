#pragma once

#include <array>
#include <utility>
#include <vector>

#include "Tomos/util/random/TRandom.hh"
#include "WorldGenConfig.hh"
#include "WorldMap.hh"

namespace NocInDan
{
    class SpawnGrid;

    // Seed + config -> WorldMap. Deterministic: the same inputs give the same tiles and spawns.
    class WorldGenerator
    {
    public:
        WorldMap generate( const WorldGenConfig& p_config );

    private:
        using Cell = std::pair<int, int>;

        struct RegionCenter
        {
            int     m_x      = 0;
            int     m_y      = 0;
            BiomeId m_biome  = BiomeId::None;
            float   m_weight = 1.0f;
        };

        void buildLandCells( const WorldGenConfig& p_config, const WorldMap& p_map );
        void layoutRegions( const WorldGenConfig& p_config );
        void addRegion( BiomeId p_biome, float p_weight );
        void assignBiomes( WorldMap& p_map );
        void assignGroundTiles( const WorldGenConfig& p_config, WorldMap& p_map ) const;
        void placePlayerSpawn( const WorldGenConfig& p_config, WorldMap& p_map );
        void placeFromTables( const WorldGenConfig& p_config, WorldMap& p_map, SpawnGrid& p_grid, SpawnPass p_pass, int p_cap );

        [[nodiscard]] const Cell& randomLandCell();
        [[nodiscard]] int         scaleByLandArea( int p_count ) const;
        [[nodiscard]] static bool isInteriorCell( const WorldMap& p_map, int p_x, int p_y, BiomeId p_biome );

        Tomos::TRandom                                                       m_rng;
        std::vector<Cell>                                                    m_landCells;
        std::vector<RegionCenter>                                            m_regions;
        std::array<std::vector<Cell>, static_cast<size_t>( BiomeId::Count )> m_cellsByBiome;
    };

}  // namespace NocInDan

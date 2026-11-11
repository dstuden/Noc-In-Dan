#include "WorldGenerator.hh"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <glm/glm.hpp>
#include <limits>
#include <utility>

#include "IslandMask.hh"
#include "Noise.hh"
#include "Tomos/util/logger/TLogger.hh"

namespace NocInDan
{
    namespace
    {
        constexpr int   g_kRegionCandidates       = 24;  // best-candidate samples per region centre
        constexpr int   g_kSpawnAttemptsPerTarget = 30;
        constexpr float g_kClumpEdge              = 0.1f;  // half-width of the soft grove edge in noise units

        // The config is authored in code: check it once here instead of guarding every use.
        void validateConfig( const WorldGenConfig& p_config )
        {
            assert( p_config.m_width >= 1 && p_config.m_width <= WorldGenConfig::kMaxMapSize && "map width out of range" );
            assert( p_config.m_height >= 1 && p_config.m_height <= WorldGenConfig::kMaxMapSize && "map height out of range" );

            [[maybe_unused]] bool anyLand  = false;
            [[maybe_unused]] bool anySpawn = false;
            for ( [[maybe_unused]] const BiomeDef& def : p_config.m_biomes )
            {
                if ( !isLandBiome( def.m_id ) ) continue;
                anyLand = true;
                anySpawn |= def.m_allowPlayerSpawn;
                assert( def.m_regionWeight > 0.0f && "land biome needs a positive region weight" );
                assert( !def.m_groundMix.empty() && "land biome needs a ground mix" );
                for ( [[maybe_unused]] const GroundMixEntry& e : def.m_groundMix ) assert( e.m_weight > 0.0f && "ground mix weights must be positive" );
            }
            assert( anyLand && "config needs at least one land biome" );
            assert( anySpawn && "config needs a land biome that allows the player spawn" );
        }
    }  // namespace

    // Spawns bucketed per map cell (intrusive lists) for min-spacing queries during placement.
    class SpawnGrid
    {
    public:
        explicit SpawnGrid( WorldMap& p_map ) : m_map( p_map ), m_head( p_map.m_tiles.size(), -1 ) {}

        bool tooClose( float p_x, float p_z, WorldUnits p_spacing ) const
        {
            const float spacing   = std::max( 0.0f, p_spacing.value );
            const int   rings     = static_cast<int>( std::ceil( spacing / m_map.m_cellSize.value ) );
            const auto [ cx, cy ] = cellOf( p_x, p_z );
            for ( int y = cy - rings; y <= cy + rings; ++y )
            {
                for ( int x = cx - rings; x <= cx + rings; ++x )
                {
                    if ( !m_map.inBounds( x, y ) ) continue;
                    for ( int i = m_head[ m_map.cellIndex( x, y ) ]; i >= 0; i = m_next[ static_cast<size_t>( i ) ] )
                    {
                        const WorldSpawn& s = m_map.m_spawns[ static_cast<size_t>( i ) ];
                        const glm::vec2   d{ s.m_x - p_x, s.m_z - p_z };
                        if ( glm::dot( d, d ) < spacing * spacing ) return true;
                    }
                }
            }
            return false;
        }

        void add( WorldSpawn p_spawn )
        {
            const auto [ cx, cy ] = cellOf( p_spawn.m_x, p_spawn.m_z );
            int& head             = m_head[ m_map.cellIndex( cx, cy ) ];
            m_next.push_back( head );
            head = static_cast<int>( m_map.m_spawns.size() );
            m_map.m_spawns.push_back( std::move( p_spawn ) );
        }

    private:
        std::pair<int, int> cellOf( float p_x, float p_z ) const
        {
            int cx = 0;
            int cy = 0;
            m_map.worldToCellUnclamped( p_x, p_z, cx, cy );
            return { std::clamp( cx, 0, m_map.m_width - 1 ), std::clamp( cy, 0, m_map.m_height - 1 ) };
        }

        WorldMap&        m_map;
        std::vector<int> m_head;
        std::vector<int> m_next;
    };

    WorldMap WorldGenerator::generate( const WorldGenConfig& p_config )
    {
        validateConfig( p_config );
        m_rng.setSeed( p_config.m_seed );
        m_regions.clear();
        for ( auto& cells : m_cellsByBiome ) cells.clear();

        WorldMap map;
        map.m_seed         = p_config.m_seed;
        map.m_width        = p_config.m_width;
        map.m_height       = p_config.m_height;
        map.m_cellSize     = p_config.m_cellSize;
        map.m_playerSpawnX = map.m_width / 2;
        map.m_playerSpawnY = map.m_height / 2;
        map.m_tiles.assign( static_cast<size_t>( map.m_width ) * static_cast<size_t>( map.m_height ), WorldTile{ BiomeId::Ocean, GroundTileId::Ocean } );

        buildLandCells( p_config, map );
        layoutRegions( p_config );
        assignBiomes( map );
        assignGroundTiles( p_config, map );
        placePlayerSpawn( p_config, map );

        SpawnGrid grid( map );
        placeFromTables( p_config, map, grid, SpawnPass::Resource, p_config.m_maxResources );
        placeFromTables( p_config, map, grid, SpawnPass::Valuable, p_config.m_maxValuables );
        placeFromTables( p_config, map, grid, SpawnPass::Threat, p_config.m_maxThreats );
        return map;
    }

    void WorldGenerator::buildLandCells( const WorldGenConfig& p_config, const WorldMap& p_map )
    {
        const std::vector<uint8_t> mask = fillIslandMask( p_map.m_width, p_map.m_height, p_config.m_seed, p_config.m_island );
        m_landCells.clear();
        for ( int y = 0; y < p_map.m_height; ++y )
            for ( int x = 0; x < p_map.m_width; ++x )
                if ( mask[ p_map.cellIndex( x, y ) ] != 0 ) m_landCells.emplace_back( x, y );

        assert( !m_landCells.empty() && "island mask params leave no land" );
    }

    const WorldGenerator::Cell& WorldGenerator::randomLandCell()
    {
        return m_landCells[ static_cast<size_t>( m_rng.nextInt( 0, static_cast<int>( m_landCells.size() ) - 1 ) ) ];
    }

    int WorldGenerator::scaleByLandArea( int p_count ) const
    {
        if ( p_count <= 0 ) return 0;
        constexpr double kMaxScale = 3.0;
        const double     scale =
                std::min( kMaxScale, static_cast<double>( m_landCells.size() ) / static_cast<double>( WorldGenConfig::kSpawnBudgetReferenceArea ) );
        return std::max( 1, static_cast<int>( std::lround( static_cast<double>( p_count ) * scale ) ) );
    }

    // Best-candidate (Mitchell) sampling: of a few random land cells, keep the one farthest from the
    // centres placed so far. Spreads centres evenly and never fails.
    void WorldGenerator::addRegion( BiomeId p_biome, float p_weight )
    {
        Cell      best     = randomLandCell();
        int       bestDist = -1;
        const int tries    = m_regions.empty() ? 0 : g_kRegionCandidates;
        for ( int i = 0; i < tries; ++i )
        {
            const Cell& c       = randomLandCell();
            int         nearest = std::numeric_limits<int>::max();
            for ( const RegionCenter& r : m_regions ) nearest = std::min( nearest, cellDistSq( c.first, c.second, r.m_x, r.m_y ) );
            if ( nearest > bestDist )
            {
                bestDist = nearest;
                best     = c;
            }
        }
        m_regions.push_back( { best.first, best.second, p_biome, p_weight } );
    }

    void WorldGenerator::layoutRegions( const WorldGenConfig& p_config )
    {
        std::vector<const BiomeDef*> land;
        float                        totalWeight = 0.0f;
        for ( const BiomeDef& def : p_config.m_biomes )
        {
            if ( !isLandBiome( def.m_id ) ) continue;
            land.push_back( &def );
            totalWeight += def.m_regionWeight;
        }

        // Every land biome once, in random order, then weighted picks up to the region count.
        m_rng.shuffle( land.begin(), land.end() );
        for ( const BiomeDef* def : land ) addRegion( def->m_id, def->m_regionWeight );

        while ( static_cast<int>( m_regions.size() ) < p_config.m_regionCount )
        {
            float           pick = m_rng.nextFloat( 0.0f, totalWeight );
            auto            it   = std::find_if( land.begin(), land.end(),
                                                 [ & ]( const BiomeDef* p_def )
                                                 {
                                        pick -= p_def->m_regionWeight;
                                        return pick <= 0.0f;
                                                 } );
            const BiomeDef* def  = it != land.end() ? *it : land.back();
            addRegion( def->m_id, def->m_regionWeight );
        }
    }

    // Weighted Voronoi: each land cell joins the centre with the smallest distance / weight.
    void WorldGenerator::assignBiomes( WorldMap& p_map )
    {
        std::vector<float> invW2( m_regions.size() );
        for ( size_t i = 0; i < m_regions.size(); ++i ) invW2[ i ] = 1.0f / ( m_regions[ i ].m_weight * m_regions[ i ].m_weight );

        for ( const auto& [ x, y ] : m_landCells )
        {
            size_t bestI     = 0;
            float  bestScore = std::numeric_limits<float>::max();
            for ( size_t i = 0; i < m_regions.size(); ++i )
            {
                const float score = static_cast<float>( cellDistSq( x, y, m_regions[ i ].m_x, m_regions[ i ].m_y ) ) * invW2[ i ];
                if ( score < bestScore )
                {
                    bestScore = score;
                    bestI     = i;
                }
            }

            const BiomeId biome        = m_regions[ bestI ].m_biome;
            p_map.tile( x, y ).m_biome = biome;
            m_cellsByBiome[ static_cast<size_t>( biome ) ].emplace_back( x, y );
        }
    }

    // Per biome: rank its cells by value noise and cut the ranking at the ground mix's weight quantiles.
    void WorldGenerator::assignGroundTiles( const WorldGenConfig& p_config, WorldMap& p_map ) const
    {
        const uint32_t baseSeed = Noise::foldSeed( p_config.m_seed, 0x7E44F00Du );

        for ( const BiomeDef& def : p_config.m_biomes )
        {
            const auto  b     = static_cast<size_t>( def.m_id );
            const auto& cells = m_cellsByBiome[ b ];
            if ( !isLandBiome( def.m_id ) || cells.empty() ) continue;

            float total = 0.0f;
            for ( const GroundMixEntry& e : def.m_groundMix ) total += e.m_weight;

            const float    freq = std::max( 0.001f, def.m_groundNoiseFreq );
            const uint32_t seed = Noise::saltSeed( baseSeed, b );

            std::vector<std::pair<float, size_t>> ranked;
            ranked.reserve( cells.size() );
            for ( size_t i = 0; i < cells.size(); ++i )
            {
                const auto [ x, y ] = cells[ i ];
                ranked.emplace_back( Noise::valueNoise2( static_cast<float>( x ) * freq, static_cast<float>( y ) * freq, seed ), i );
            }
            std::sort( ranked.begin(), ranked.end() );

            size_t start      = 0;
            float  cumulative = 0.0f;
            for ( size_t e = 0; e < def.m_groundMix.size(); ++e )
            {
                cumulative += def.m_groundMix[ e ].m_weight;
                const bool   last = e + 1 == def.m_groundMix.size();
                const size_t end =
                        last ? ranked.size()
                             : std::min( ranked.size(), static_cast<size_t>( std::lround( cumulative / total * static_cast<float>( ranked.size() ) ) ) );
                for ( size_t r = start; r < end; ++r )
                {
                    const auto [ x, y ]         = cells[ ranked[ r ].second ];
                    p_map.tile( x, y ).m_ground = def.m_groundMix[ e ].m_tile;
                }
                start = std::max( start, end );
            }
        }
    }

    void WorldGenerator::placePlayerSpawn( const WorldGenConfig& p_config, WorldMap& p_map )
    {
        std::vector<Cell> candidates;
        for ( const BiomeDef& def : p_config.m_biomes )
        {
            if ( !def.m_allowPlayerSpawn || !isLandBiome( def.m_id ) ) continue;
            const auto& cells = m_cellsByBiome[ static_cast<size_t>( def.m_id ) ];
            candidates.insert( candidates.end(), cells.begin(), cells.end() );
        }
        // A spawn biome can lose all its cells to stronger neighbours on a tiny island.
        if ( candidates.empty() ) candidates = m_landCells;

        const auto& [ sx, sy ] = candidates[ static_cast<size_t>( m_rng.nextInt( 0, static_cast<int>( candidates.size() ) - 1 ) ) ];
        p_map.m_playerSpawnX   = sx;
        p_map.m_playerSpawnY   = sy;
    }

    bool WorldGenerator::isInteriorCell( const WorldMap& p_map, int p_x, int p_y, BiomeId p_biome )
    {
        if ( p_x <= 0 || p_y <= 0 || p_x >= p_map.m_width - 1 || p_y >= p_map.m_height - 1 ) return false;
        return p_map.tile( p_x + 1, p_y ).m_biome == p_biome && p_map.tile( p_x - 1, p_y ).m_biome == p_biome &&
               p_map.tile( p_x, p_y + 1 ).m_biome == p_biome && p_map.tile( p_x, p_y - 1 ).m_biome == p_biome;
    }

    void WorldGenerator::placeFromTables( const WorldGenConfig& p_config, WorldMap& p_map, SpawnGrid& p_grid, SpawnPass p_pass, int p_cap )
    {
        int remaining = scaleByLandArea( p_cap );

        const float    cellSize = p_map.m_cellSize.value;
        const uint32_t baseSeed = Noise::foldSeed( p_config.m_seed, 0xC1A3F5EDu );

        std::vector<Cell> eligible;
        for ( const BiomeDef& def : p_config.m_biomes )
        {
            if ( !isLandBiome( def.m_id ) ) continue;
            const auto  biomeIdx   = static_cast<size_t>( def.m_id );
            const auto& biomeCells = m_cellsByBiome[ biomeIdx ];

            for ( const SpawnEntry& entry : def.m_spawns )
            {
                if ( remaining <= 0 ) return;
                if ( entry.m_pass != p_pass || entry.m_prefabId.empty() || entry.m_density <= 0.0f ) continue;

                eligible.clear();
                for ( const auto& [ x, y ] : biomeCells )
                {
                    if ( entry.m_avoidPlayerSpawn && withinPlayerClear( x, y, p_map.m_playerSpawnX, p_map.m_playerSpawnY, p_config.m_playerClearRadius ) )
                        continue;
                    if ( entry.m_requireInterior && !isInteriorCell( p_map, x, y, def.m_id ) ) continue;
                    eligible.emplace_back( x, y );
                }
                if ( eligible.empty() ) continue;

                const int      target    = std::min( remaining, static_cast<int>( std::lround( entry.m_density * static_cast<float>( eligible.size() ) ) ) );
                const uint32_t clumpSeed = Noise::saltSeed( baseSeed, biomeIdx ) ^ Noise::hashString( entry.m_prefabId );
                const int      lastCell  = static_cast<int>( eligible.size() ) - 1;

                // rejection sampling keeps the target count under clumping
                int placed = 0;
                for ( int a = 0; a < target * g_kSpawnAttemptsPerTarget && placed < target; ++a )
                {
                    const auto [ cx, cy ] = eligible[ static_cast<size_t>( m_rng.nextInt( 0, lastCell ) ) ];
                    const float px        = p_map.cellWorldX( cx ) + m_rng.nextFloat( -0.5f, 0.5f ) * cellSize;
                    const float pz        = p_map.cellWorldZ( cy ) + m_rng.nextFloat( -0.5f, 0.5f ) * cellSize;

                    if ( entry.m_clumpFreq > 0.0f )
                    {
                        const float f = entry.m_clumpFreq / cellSize;
                        const float n = Noise::valueNoise2( px * f, pz * f, clumpSeed );
                        if ( !m_rng.nextBool( glm::smoothstep( entry.m_clumpCutoff - g_kClumpEdge, entry.m_clumpCutoff + g_kClumpEdge, n ) ) ) continue;
                    }
                    if ( p_grid.tooClose( px, pz, entry.m_minSpacing ) ) continue;

                    p_grid.add( { entry.m_prefabId, px, pz } );
                    ++placed;
                }

                remaining -= placed;
                TLOG_DEBUG() << "[WorldGenerator] " << entry.m_prefabId << " biome " << biomeIdx << ": " << placed << "/" << target;
            }
        }
    }

}  // namespace NocInDan

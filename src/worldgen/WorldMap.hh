#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#include "../utils/Biome.hh"
#include "GroundTileId.hh"
#include "SpatialUnits.hh"

namespace NocInDan
{

    struct WorldTile
    {
        BiomeId      m_biome  = BiomeId::None;
        GroundTileId m_ground = GroundTileId::None;
    };

    struct WorldSpawn
    {
        std::string m_prefabId;
        float       m_x = 0.0f;
        float       m_z = 0.0f;
    };

    // A generated world: the tile grid (centred on the world origin) and the spawn list.
    // Everything here follows from the seed and the config, so none of it is saved.
    struct WorldMap
    {
        // Ground meshes and spawn batches are built per chunk of this many cells a side.
        static constexpr int kChunkCells = 16;

        uint64_t   m_seed   = 0;
        int        m_width  = 0;
        int        m_height = 0;
        WorldUnits m_cellSize{ 8.0f };

        std::vector<WorldTile> m_tiles;  // row-major: index = y * width + x
        // Index = spawn id: stable for a given seed and config, so saves can name removed spawns.
        std::vector<WorldSpawn> m_spawns;

        int m_playerSpawnX = 0;
        int m_playerSpawnY = 0;

        size_t     cellIndex( int p_x, int p_y ) const { return static_cast<size_t>( p_y ) * static_cast<size_t>( m_width ) + static_cast<size_t>( p_x ); }
        WorldTile& tile( int p_x, int p_y ) { return m_tiles[ cellIndex( p_x, p_y ) ]; }
        const WorldTile& tile( int p_x, int p_y ) const { return m_tiles[ cellIndex( p_x, p_y ) ]; }
        bool             inBounds( int p_x, int p_y ) const { return p_x >= 0 && p_y >= 0 && p_x < m_width && p_y < m_height; }

        float cellWorldX( int p_x ) const { return ( static_cast<float>( p_x ) + 0.5f - 0.5f * static_cast<float>( m_width ) ) * m_cellSize.value; }
        float cellWorldZ( int p_y ) const { return ( static_cast<float>( p_y ) + 0.5f - 0.5f * static_cast<float>( m_height ) ) * m_cellSize.value; }

        // inverse of cellWorldX/Z; may be out of range
        void worldToCellUnclamped( float p_x, float p_z, int& p_cx, int& p_cy ) const
        {
            const float s = m_cellSize.value;
            p_cx          = static_cast<int>( std::floor( p_x / s + 0.5f * static_cast<float>( m_width ) ) );
            p_cy          = static_cast<int>( std::floor( p_z / s + 0.5f * static_cast<float>( m_height ) ) );
        }

        float worldWidth() const { return ( m_width * m_cellSize ).value; }
        float worldDepth() const { return ( m_height * m_cellSize ).value; }

        // Off-map cells are impassable.
        const TileMovementRules& rulesAt( int p_x, int p_y ) const
        {
            return movementRules( inBounds( p_x, p_y ) ? tile( p_x, p_y ).m_ground : GroundTileId::None );
        }
        bool passableAt( int p_x, int p_y ) const { return rulesAt( p_x, p_y ).m_passable; }

        const TileMovementRules& rulesAtPoint( float p_x, float p_z ) const
        {
            int cx = 0;
            int cy = 0;
            worldToCellUnclamped( p_x, p_z, cx, cy );
            return rulesAt( cx, cy );
        }
        bool isPassableAtPoint( float p_x, float p_z ) const { return rulesAtPoint( p_x, p_z ).m_passable; }

        static constexpr int chunkCount( int p_cells ) { return ( p_cells + kChunkCells - 1 ) / kChunkCells; }
        int                  chunkCountX() const { return chunkCount( m_width ); }
        int                  chunkCountY() const { return chunkCount( m_height ); }

        // Chunk index (y * chunkCountX + x) holding world point (p_x, p_z), clamped to the map.
        size_t chunkIndexAt( float p_x, float p_z ) const
        {
            int cx = 0;
            int cy = 0;
            worldToCellUnclamped( p_x, p_z, cx, cy );
            const int chunkX = std::clamp( cx, 0, m_width - 1 ) / kChunkCells;
            const int chunkY = std::clamp( cy, 0, m_height - 1 ) / kChunkCells;
            return static_cast<size_t>( chunkY * chunkCountX() + chunkX );
        }
    };

}  // namespace NocInDan

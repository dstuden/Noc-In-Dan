#include "WorldGroundMesh.hh"

#include <algorithm>
#include <cstring>
#include <vector>

#include "CellRects.hh"
#include "Tomos/gpu/TRenderLimits.hh"
#include "Tomos/systems/asset/TGltfLoader.hh"

namespace NocInDan
{
    namespace
    {
        constexpr glm::vec3 g_kUp{ 0.0f, 1.0f, 0.0f };

        // Corner-grid bounds of a chunk: cells [x0, x1) x [y0, y1), vertices [x0, x1] x [y0, y1].
        CellRect chunkCells( const WorldMap& p_world, int p_chunkX, int p_chunkY )
        {
            return { p_chunkX * WorldMap::kChunkCells, p_chunkY * WorldMap::kChunkCells, std::min( ( p_chunkX + 1 ) * WorldMap::kChunkCells, p_world.m_width ),
                     std::min( ( p_chunkY + 1 ) * WorldMap::kChunkCells, p_world.m_height ) };
        }

        bool isLandCell( const WorldMap& p_world, int p_x, int p_y )
        {
            return p_world.inBounds( p_x, p_y ) && isLandGround( p_world.tile( p_x, p_y ).m_ground );
        }

        std::unique_ptr<Tomos::TVkMesh> upload( Tomos::TVkGpu& p_gpu, Tomos::TCpuMeshData& p_mesh )
        {
            p_mesh.m_normals.assign( p_mesh.m_positions.size(), g_kUp );
            p_mesh.m_tangents.assign( p_mesh.m_positions.size(), glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
            p_mesh.m_is16BitIndex = false;
            p_mesh.m_drawCount    = static_cast<uint32_t>( p_mesh.m_indices32.size() );
            p_mesh.m_aabb         = { p_mesh.m_positions.front(), p_mesh.m_positions.front() };
            for ( const glm::vec3& p : p_mesh.m_positions )
            {
                p_mesh.m_aabb.m_min = glm::min( p_mesh.m_aabb.m_min, p );
                p_mesh.m_aabb.m_max = glm::max( p_mesh.m_aabb.m_max, p );
            }
            return Tomos::TGltfLoader::uploadMesh( p_mesh, p_gpu );
        }
    }  // namespace

    std::vector<std::byte> buildGroundShaderData( const WorldMap& p_world, std::span<const GroundLayerGpu> p_layers )
    {
        struct Header
        {
            uint32_t m_width, m_height, m_pad0, m_pad1;
        };
        const size_t layerBytes = Tomos::g_kMaxGroundLayers * sizeof( GroundLayerGpu );
        const size_t idWords    = ( p_world.m_tiles.size() + 3 ) / 4;

        std::vector<std::byte> data( sizeof( Header ) + layerBytes + idWords * sizeof( uint32_t ) );
        const Header           header{ static_cast<uint32_t>( p_world.m_width ), static_cast<uint32_t>( p_world.m_height ), 0, 0 };
        std::memcpy( data.data(), &header, sizeof( header ) );
        std::memcpy( data.data() + sizeof( Header ), p_layers.data(), std::min( p_layers.size_bytes(), layerBytes ) );

        std::byte* ids = data.data() + sizeof( Header ) + layerBytes;
        for ( size_t i = 0; i < p_world.m_tiles.size(); ++i ) ids[ i ] = static_cast<std::byte>( p_world.m_tiles[ i ].m_ground );
        return data;
    }

    std::unique_ptr<Tomos::TVkMesh> makeLandChunkMesh( Tomos::TVkGpu& p_gpu, const WorldMap& p_world, int p_chunkX, int p_chunkY )
    {
        const CellRect r    = chunkCells( p_world, p_chunkX, p_chunkY );
        const int      cols = r.m_x1 - r.m_x0 + 1;
        const float    half = p_world.m_cellSize.value * 0.5f;

        Tomos::TCpuMeshData mesh;
        for ( int j = r.m_y0; j <= r.m_y1; ++j )
        {
            for ( int i = r.m_x0; i <= r.m_x1; ++i )
            {
                mesh.m_positions.emplace_back( p_world.cellWorldX( i ) - half, 0.0f, p_world.cellWorldZ( j ) - half );
                mesh.m_uvs.emplace_back( static_cast<float>( i ), static_cast<float>( j ) );
            }
        }

        for ( int cy = r.m_y0; cy < r.m_y1; ++cy )
        {
            for ( int cx = r.m_x0; cx < r.m_x1; ++cx )
            {
                if ( !isLandCell( p_world, cx, cy ) ) continue;
                const auto d = static_cast<uint32_t>( ( cy - r.m_y0 ) * cols + ( cx - r.m_x0 ) );
                const auto c = d + 1;
                const auto a = d + static_cast<uint32_t>( cols );
                const auto b = a + 1;
                mesh.m_indices32.insert( mesh.m_indices32.end(), { a, b, c, a, c, d } );
            }
        }
        if ( mesh.m_indices32.empty() ) return nullptr;
        return upload( p_gpu, mesh );
    }

    std::unique_ptr<Tomos::TVkMesh> makeOceanMesh( Tomos::TVkGpu& p_gpu, const WorldMap& p_world )
    {
        const float         half = p_world.m_cellSize.value * 0.5f;
        Tomos::TCpuMeshData mesh;
        for ( const CellRect& r : mergeCellRects( p_world.m_width, p_world.m_height, [ & ]( int p_x, int p_y ) { return !isLandCell( p_world, p_x, p_y ); } ) )
        {
            const float x0   = p_world.cellWorldX( r.m_x0 ) - half;
            const float x1   = p_world.cellWorldX( r.m_x1 ) + half;
            const float z0   = p_world.cellWorldZ( r.m_y0 ) - half;
            const float z1   = p_world.cellWorldZ( r.m_y1 ) + half;
            const auto  base = static_cast<uint32_t>( mesh.m_positions.size() );
            mesh.m_positions.insert( mesh.m_positions.end(), { { x0, 0.0f, z0 }, { x1, 0.0f, z0 }, { x0, 0.0f, z1 }, { x1, 0.0f, z1 } } );
            const float u0 = static_cast<float>( r.m_x0 ), u1 = static_cast<float>( r.m_x1 + 1 );
            const float v0 = static_cast<float>( r.m_y0 ), v1 = static_cast<float>( r.m_y1 + 1 );
            mesh.m_uvs.insert( mesh.m_uvs.end(), { { u0, v0 }, { u1, v0 }, { u0, v1 }, { u1, v1 } } );
            mesh.m_indices32.insert( mesh.m_indices32.end(), { base + 2, base + 3, base + 1, base + 2, base + 1, base + 0 } );
        }
        if ( mesh.m_indices32.empty() ) return nullptr;
        return upload( p_gpu, mesh );
    }

}  // namespace NocInDan

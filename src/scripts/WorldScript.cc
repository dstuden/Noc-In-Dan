#include "WorldScript.hh"

#include <algorithm>
#include <string>

#include "../prefabs/PrefabBuilder.hh"
#include "../scenes/InGameScene.hh"
#include "../worldgen/WorldGenConfig.hh"
#include "../worldgen/WorldGenerator.hh"
#include "../worldgen/WorldGeometry.hh"
#include "Tomos/core/app/TApplication.hh"
#include "Tomos/core/scene/TScene.hh"
#include "Tomos/gpu/vulkan/TVkGpu.hh"
#include "Tomos/util/logger/TLogger.hh"

namespace NocInDan
{
    void WorldScript::onAttach()
    {
        auto& scene = static_cast<Tomos::TScene&>( node().root() );
        auto* gpu   = Tomos::TApplication::get().gpu();

        m_map = std::make_unique<WorldMap>( WorldGenerator{}.generate( makeDefaultWorldGenConfig( m_seed ) ) );

        Tomos::TSceneNode& geometry = scene.createNode( "WorldGeometry" );
        geometry.m_transient        = true;
        node().addChild( &geometry );
        buildWorldGeometry( scene, *gpu, geometry, *m_map );

        // Spawns grouped by chunk, so removing one rebuilds a chunk rather than the world.
        const size_t chunks = static_cast<size_t>( m_map->chunkCountX() * m_map->chunkCountY() );
        m_chunkSpawns.assign( chunks, {} );
        for ( uint32_t id = 0; id < m_map->m_spawns.size(); ++id )
        {
            if ( std::ranges::binary_search( m_removedSpawns, id ) ) continue;
            const WorldSpawn& s = m_map->m_spawns[ id ];
            m_chunkSpawns[ m_map->chunkIndexAt( s.m_x, s.m_z ) ].push_back( id );
        }

        Tomos::TSceneNode& spawns = scene.createNode( "Spawns" );
        spawns.m_transient        = true;
        node().addChild( &spawns );
        m_spawnRoot = spawns.handle();
        m_chunkNodes.assign( chunks, {} );
        for ( size_t c = 0; c < chunks; ++c ) buildChunk( c );

        m_scene = InGameScene::of( node() );
        if ( m_scene != nullptr ) m_scene->m_world = this;

        TLOG_INFO() << "[WorldScript] world " << m_map->m_width << "x" << m_map->m_height << " seed=" << m_seed << " spawns=" << m_map->m_spawns.size()
                    << " removed=" << m_removedSpawns.size();
    }

    void WorldScript::onDetach()
    {
        if ( m_scene != nullptr && m_scene->m_world == this ) m_scene->m_world = nullptr;
    }

    void WorldScript::buildChunk( size_t p_chunk )
    {
        auto&              scene = static_cast<Tomos::TScene&>( node().root() );
        Tomos::TSceneNode* root  = scene.store().getNode( m_spawnRoot );
        if ( root == nullptr ) return;

        if ( m_chunkNodes[ p_chunk ] ) scene.store().destroyNode( m_chunkNodes[ p_chunk ] );
        m_chunkNodes[ p_chunk ] = {};
        if ( m_chunkSpawns[ p_chunk ].empty() ) return;

        Tomos::TSceneNode& chunk = scene.createNode( "Chunk " + std::to_string( p_chunk ) );
        root->addChild( &chunk );
        m_chunkNodes[ p_chunk ] = chunk.handle();
        buildSpawnChunk( scene, *Tomos::TApplication::get().gpu(), chunk, *m_map, m_chunkSpawns[ p_chunk ] );
    }

    void WorldScript::removeSpawn( uint32_t p_id )
    {
        if ( m_map == nullptr || p_id >= m_map->m_spawns.size() ) return;
        const auto at = std::ranges::lower_bound( m_removedSpawns, p_id );
        if ( at != m_removedSpawns.end() && *at == p_id ) return;
        m_removedSpawns.insert( at, p_id );

        const WorldSpawn& s     = m_map->m_spawns[ p_id ];
        const size_t      chunk = m_map->chunkIndexAt( s.m_x, s.m_z );
        std::erase( m_chunkSpawns[ chunk ], p_id );
        buildChunk( chunk );
    }
}  // namespace NocInDan

#include "PrefabBuilder.hh"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../physics/StaticObstacle.hh"
#include "PrefabCatalog.hh"
#include "Tomos/core/scene/TScene.hh"
#include "Tomos/core/scene/TSceneNode.hh"
#include "Tomos/gpu/TBillboardMode.hh"
#include "Tomos/gpu/TGpuEnums.hh"
#include "Tomos/gpu/TSpriteAlphaMode.hh"
#include "Tomos/gpu/vulkan/TVkGpu.hh"
#include "Tomos/systems/physics/TColliderComponent.hh"
#include "Tomos/systems/sprite/TSpriteInstancesComponent.hh"
#include "Tomos/util/logger/TLogger.hh"

namespace NocInDan
{
    namespace
    {
        const Tomos::TVkImage* resolveTexture( Tomos::TScene& p_scene, Tomos::TVkGpu& p_gpu, const std::string& p_path )
        {
            if ( p_path.empty() ) return &p_gpu.defaultTexture();
            const Tomos::TVkImage* img = p_scene.resources().loadImage( p_gpu, p_path, Tomos::TTexAddr::Clamp, /*p_required=*/false );
            return img == nullptr || img == &p_gpu.missingTexture() ? &p_gpu.defaultTexture() : img;
        }

        // Upright cutout card standing on its node/instance position.
        void applyStyle( Tomos::TSpriteComponent& p_sprite, const PrefabDef& p_def )
        {
            p_sprite.m_size                = p_def.m_size;
            p_sprite.m_color               = p_def.m_color;
            p_sprite.m_pivot               = { 0.5f, 0.0f };
            p_sprite.m_mode                = Tomos::TBillboardMode::Cylindrical;
            p_sprite.m_alphaMode           = Tomos::TSpriteAlphaMode::Cutout;
            p_sprite.m_shadowMode          = p_def.m_shadow;
            p_sprite.m_shadowContactRadius = p_def.m_contactShadowRadius;
        }

        const PrefabDef* findPrefab( const std::string& p_id )
        {
            const PrefabDef*                       def = PrefabCatalog::get().find( p_id );
            static std::unordered_set<std::string> s_warned;
            if ( def == nullptr && s_warned.insert( p_id ).second ) TLOG_WARN() << "[PrefabBuilder] unknown prefab '" << p_id << "'";
            return def;
        }
    }  // namespace

    void buildSpawnChunk( Tomos::TScene& p_scene, Tomos::TVkGpu& p_gpu, Tomos::TSceneNode& p_parent, const WorldMap& p_map, std::span<const uint32_t> p_ids )
    {
        std::unordered_map<const PrefabDef*, std::vector<glm::vec3>> decoration;

        for ( const uint32_t id : p_ids )
        {
            const WorldSpawn& spawn = p_map.m_spawns[ id ];
            const PrefabDef*  def   = findPrefab( spawn.m_prefabId );
            if ( def == nullptr ) continue;

            const glm::vec3 pos{ spawn.m_x, 0.0f, spawn.m_z };
            if ( !def->m_hasCollider )
            {
                decoration[ def ].push_back( pos );
                continue;
            }

            Tomos::TSceneNode& node = p_scene.createNode( def->m_id );
            node.m_transform.setTranslation( pos );
            p_parent.addChild( &node );

            auto& sprite = node.emplaceComponent<Tomos::TSpriteComponent>( resolveTexture( p_scene, p_gpu, def->m_texturePath ) );
            applyStyle( sprite, *def );
            sprite.m_static = true;

            auto& col        = node.emplaceComponent<Tomos::TColliderComponent>();
            col.m_shape      = Tomos::TColliderShape::Cylinder;
            col.m_radius     = def->m_colliderRadius;
            col.m_halfHeight = def->m_colliderHalfHeight;
            configureStaticObstacle( col );
        }

        for ( auto& [ def, positions ] : decoration )
        {
            Tomos::TSceneNode& node = p_scene.createNode( def->m_id );
            p_parent.addChild( &node );

            auto& batch = node.emplaceComponent<Tomos::TSpriteInstancesComponent>( resolveTexture( p_scene, p_gpu, def->m_texturePath ) );
            applyStyle( batch, *def );
            batch.setPositions( std::move( positions ) );
        }
    }

}  // namespace NocInDan

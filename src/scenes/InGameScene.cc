#include "InGameScene.hh"

#include <cstdint>
#include <memory>

#include "../components/TiltedCameraComponent.hh"
#include "../physics/StaticObstacle.hh"
#include "../scripts/MobScript.hh"
#include "../scripts/PlayerScript.hh"
#include "../scripts/SunOrbitScript.hh"
#include "../scripts/WorldScript.hh"
#include "Tomos/core/app/TApplication.hh"
#include "Tomos/gpu/vulkan/TVkMaterial.hh"
#include "Tomos/systems/asset/TAssetSystem.hh"
#include "Tomos/systems/light/TLightComponent.hh"
#include "Tomos/systems/mesh/TMeshComponent.hh"
#include "Tomos/systems/mesh/TMeshPrimitives.hh"
#include "Tomos/systems/physics/TPhysicsLayers.hh"
#include "Tomos/systems/post/TColorGradeComponent.hh"
#include "Tomos/systems/script/TScriptComponent.hh"
#include "Tomos/systems/spriterig/TSpriteRigComponent.hh"
#include "Tomos/util/random/TRandom.hh"

namespace NocInDan
{
    InGameScene::InGameScene() : Tomos::TScene( "InGame" ) {}

    InGameScene* InGameScene::of( Tomos::TSceneNode& p_node ) { return dynamic_cast<InGameScene*>( &p_node.root() ); }

    const WorldMap* InGameScene::worldMap() const { return m_world != nullptr ? m_world->map() : nullptr; }

    const Survival* InGameScene::survival() const { return m_player != nullptr ? &m_player->m_survival : nullptr; }

    const WorldClock* InGameScene::clock() const { return m_sun != nullptr ? &m_sun->m_clock : nullptr; }

    void InGameScene::onSystemsReady()
    {
        createPlayer();
        createWorld();
        spawnMobs();
        createLight();
    }

    void InGameScene::createPlayer()
    {
        auto& app = Tomos::TApplication::get();

        Tomos::TSceneNode& player = createNode( "Player" );
        player.m_transform.setTranslation( { 0.0f, 2.0f, 0.0f } );
        addChild( &player );

        namespace L = Tomos::TPhysicsLayer;
        makeDynamicSphere( player, 0.5f, L::g_player, L::g_static | L::g_dynamic | L::g_default );

        // placeholder body
        Tomos::TVkMaterialDesc mat{};
        mat.m_baseColorFactor = glm::vec4( 0.95f, 0.35f, 0.2f, 1.0f );
        mat.m_roughnessFactor = 0.55f;
        mat.m_emissionFactor  = glm::vec3( 0.95f, 0.35f, 0.2f ) * 0.08f;
        if ( const Tomos::TGpuAsset* box = Tomos::TMeshPrimitives::boxAsset( *app.gpu(), app.assetSystem(), "NocInDan.Player", mat ) )
            player.emplaceComponent<Tomos::TMeshComponent>( box->mesh( 0 ), box->material( 0 ) );

        Tomos::TSceneNode& camera = createNode( "Camera" );
        addChild( &camera );
        camera.emplaceComponent<TiltedCameraComponent>().setTarget( &player );

        // after the camera: PlayerScript snaps it on attach
        auto script = std::make_unique<PlayerScript>( camera.handle() );
        script->m_survival.m_inventory.add( 1, 6 );
        script->m_survival.m_inventory.add( 2, 3 );
        script->m_survival.m_inventory.add( 3, 12 );
        player.addComponent( std::make_unique<Tomos::TScriptComponent>( std::move( script ) ) );
    }

    void InGameScene::createWorld()
    {
        // New game: a fresh seed. WorldScript builds everything derived from it on attach.
        const uint64_t seed = Tomos::TRandom::global().nextUInt64( 0, UINT64_MAX );

        Tomos::TSceneNode& world = createNode( "World" );
        addChild( &world );
        world.addComponent( Tomos::TScriptComponent::make<WorldScript>( seed ) );

        const WorldMap& map = *worldMap();
        m_player->node().m_transform.setTranslation( { map.cellWorldX( map.m_playerSpawnX ), 2.0f, map.cellWorldZ( map.m_playerSpawnY ) } );
    }

    void InGameScene::spawnMobs()
    {
        constexpr const char* kRig      = "assets/rigs/pig.json";
        constexpr float       kRadius   = 0.4f;
        const glm::vec3       origin    = m_player->node().m_transform.translation();
        const glm::vec2       offsets[] = { { 4.0f, 3.0f }, { -5.0f, 2.0f }, { 2.0f, -5.0f } };

        for ( const glm::vec2& o : offsets )
        {
            const glm::vec3 pos{ origin.x + o.x, kRadius + 0.1f, origin.z + o.y };
            if ( !worldMap()->isPassableAtPoint( pos.x, pos.z ) ) continue;

            Tomos::TSceneNode& mob = createNode( "Pig" );
            addChild( &mob );
            mob.m_transform.setTranslation( pos );

            namespace L = Tomos::TPhysicsLayer;
            makeDynamicSphere( mob, kRadius, L::g_dynamic, L::g_static | L::g_player | L::g_dynamic | L::g_default );

            mob.emplaceComponent<Tomos::TSpriteRigComponent>().m_rigPath = kRig;
            mob.addComponent( Tomos::TScriptComponent::make<MobScript>() );
        }
    }

    void InGameScene::createLight()
    {
        Tomos::TSceneNode& sun = createNode( "Sun" );
        addChild( &sun );
        sun.emplaceComponent<Tomos::TLightComponent>().m_type = Tomos::TLightType::Directional;
        sun.emplaceComponent<Tomos::TColorGradeComponent>();

        Tomos::TSceneNode& ambient = createNode( "Ambient" );
        sun.addChild( &ambient );
        ambient.emplaceComponent<Tomos::TLightComponent>().m_type = Tomos::TLightType::Ambient;

        // last: it finds the lights and grade on attach and applies the current time of day
        sun.addComponent( Tomos::TScriptComponent::make<SunOrbitScript>() );
    }
}  // namespace NocInDan

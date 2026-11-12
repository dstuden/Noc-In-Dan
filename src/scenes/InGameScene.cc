#include "InGameScene.hh"

#include <cstdint>
#include <memory>

#include "../components/TiltedCameraComponent.hh"
#include "../physics/StaticObstacle.hh"
#include "../scripts/CampfireScript.hh"
#include "../scripts/MobScript.hh"
#include "../scripts/PlayerScript.hh"
#include "../scripts/SunOrbitScript.hh"
#include "../scripts/WorldScript.hh"
#include "Tomos/core/app/TApplication.hh"
#include "Tomos/gpu/TBillboardMode.hh"
#include "Tomos/gpu/TGpuEnums.hh"
#include "Tomos/gpu/TSpriteAlphaMode.hh"
#include "Tomos/gpu/TSpriteShadowMode.hh"
#include "Tomos/gpu/vulkan/TVkGpu.hh"
#include "Tomos/gpu/vulkan/TVkMaterial.hh"
#include "Tomos/systems/asset/TAssetSystem.hh"
#include "Tomos/systems/light/TLightComponent.hh"
#include "Tomos/systems/mesh/TMeshComponent.hh"
#include "Tomos/systems/mesh/TMeshPrimitives.hh"
#include "Tomos/systems/particle/TParticleEmitterComponent.hh"
#include "Tomos/systems/physics/TColliderComponent.hh"
#include "Tomos/systems/physics/TPhysicsLayers.hh"
#include "Tomos/systems/post/TColorGradeComponent.hh"
#include "Tomos/systems/script/TScriptComponent.hh"
#include "Tomos/systems/sprite/TSpriteComponent.hh"
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
        createCampfire();
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

    void InGameScene::createCampfire()
    {
        constexpr const char* kSheet       = "assets/prefabs/campfire.png";
        constexpr int         kSheetFrames = 6;

        auto&           app   = Tomos::TApplication::get();
        const WorldMap& map   = *worldMap();
        const glm::vec3 spawn = m_player->node().m_transform.translation();

        constexpr glm::vec2 kOffsets[] = { { 2.5f, 1.5f }, { -2.5f, 1.5f }, { 2.5f, -1.5f }, { -2.5f, -1.5f }, { 0.0f, 3.0f } };
        glm::vec3           pos{ spawn.x + kOffsets[ 0 ].x, 0.0f, spawn.z + kOffsets[ 0 ].y };
        for ( const glm::vec2& o : kOffsets )
        {
            if ( !map.isPassableAtPoint( spawn.x + o.x, spawn.z + o.y ) ) continue;
            pos = { spawn.x + o.x, 0.0f, spawn.z + o.y };
            break;
        }

        Tomos::TSceneNode& fire = createNode( "Campfire" );
        addChild( &fire );
        fire.m_transform.setTranslation( pos );

        auto& col        = fire.emplaceComponent<Tomos::TColliderComponent>();
        col.m_shape      = Tomos::TColliderShape::Cylinder;
        col.m_radius     = 0.55f;
        col.m_halfHeight = 0.5f;
        configureStaticObstacle( col );

        // TODO: make spritesheet animation a built-in component, so we don't have to write a script for every one.
        const Tomos::TVkImage* tex = resources().loadImage( *app.gpu(), kSheet, Tomos::TTexAddr::Clamp, /*p_required=*/false );
        if ( tex == nullptr ) tex = &app.gpu()->defaultTexture();

        auto& sprite                 = fire.emplaceComponent<Tomos::TSpriteComponent>( tex );
        sprite.m_animRef.m_path      = kSheet;
        sprite.m_size                = { 1.4f, 1.4f };
        sprite.m_pivot               = { 0.5f, 0.0f };
        sprite.m_uvMax               = { 1.0f / static_cast<float>( kSheetFrames ), 1.0f };
        sprite.m_mode                = Tomos::TBillboardMode::Cylindrical;
        sprite.m_alphaMode           = Tomos::TSpriteAlphaMode::Cutout;
        sprite.m_shadowMode          = Tomos::TSpriteShadowMode::Contact;
        sprite.m_shadowContactRadius = 0.35f;
        sprite.m_emissive            = 1.0f;  // flames are their own light source

        Tomos::TSceneNode& lightNode = createNode( "CampfireLight" );
        fire.addChild( &lightNode );
        lightNode.m_transform.setTranslation( { 0.0f, 0.6f, 0.0f } );
        auto& light        = lightNode.emplaceComponent<Tomos::TLightComponent>();
        light.m_type       = Tomos::TLightType::Point;
        light.m_color      = { 1.0f, 0.6f, 0.25f };
        light.m_intensity  = 6.0f;
        light.m_maxRange   = 8.0f;
        light.m_castShadow = false;

        Tomos::TSceneNode& embersNode = createNode( "CampfireEmbers" );
        fire.addChild( &embersNode );
        embersNode.m_transform.setTranslation( { 0.0f, 0.55f, 0.0f } );
        auto& embers         = embersNode.emplaceComponent<Tomos::TParticleEmitterComponent>();
        embers.m_rate        = 14.0f;
        embers.m_lifetimeMin = 0.7f;
        embers.m_lifetimeMax = 1.6f;
        embers.m_velocityMin = { -0.35f, 0.9f, -0.35f };
        embers.m_velocityMax = { 0.35f, 2.0f, 0.35f };
        embers.m_sizeStart   = { 0.07f, 0.07f };
        embers.m_sizeEnd     = { 0.015f, 0.015f };
        embers.m_colorStart  = { 1.0f, 0.65f, 0.2f, 1.0f };
        embers.m_colorEnd    = { 1.0f, 0.15f, 0.0f, 0.0f };
        embers.m_gravity     = -0.5f;
        embers.m_seed        = 1;

        Tomos::TSceneNode& glowNode = createNode( "CampfireGlow" );
        fire.addChild( &glowNode );
        glowNode.m_transform.setTranslation( { 0.0f, 0.35f, 0.0f } );
        auto& glow         = glowNode.emplaceComponent<Tomos::TParticleEmitterComponent>();
        glow.m_rate        = 24.0f;
        glow.m_lifetimeMin = 0.25f;
        glow.m_lifetimeMax = 0.5f;
        glow.m_velocityMin = { -0.15f, 0.6f, -0.15f };
        glow.m_velocityMax = { 0.15f, 1.4f, 0.15f };
        glow.m_sizeStart   = { 0.8f, 0.8f };
        glow.m_sizeEnd     = { 0.12f, 0.12f };
        glow.m_colorStart  = { 1.0f, 0.55f, 0.15f, 0.35f };
        glow.m_colorEnd    = { 1.0f, 0.25f, 0.05f, 0.0f };
        glow.m_gravity     = 0.0f;
        glow.m_seed        = 2;

        auto script          = std::make_unique<CampfireScript>();
        script->m_frameCount = kSheetFrames;
        fire.addComponent( std::make_unique<Tomos::TScriptComponent>( std::move( script ) ) );
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

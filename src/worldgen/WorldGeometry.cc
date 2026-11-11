#include "WorldGeometry.hh"

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "../physics/StaticObstacle.hh"
#include "CoastWalls.hh"
#include "GroundTileDef.hh"
#include "Tomos/core/app/TApplication.hh"
#include "Tomos/core/scene/TScene.hh"
#include "Tomos/gpu/TRenderLimits.hh"
#include "Tomos/gpu/vulkan/TMeshTechnique.hh"
#include "Tomos/gpu/vulkan/TVkGpu.hh"
#include "Tomos/gpu/vulkan/TVkMaterial.hh"
#include "Tomos/gpu/vulkan/renderer/TVkClusteredRenderer.hh"
#include "Tomos/systems/asset/TAssetSystem.hh"
#include "Tomos/systems/mesh/TMeshComponent.hh"
#include "Tomos/util/logger/TLogger.hh"
#include "WorldGroundMesh.hh"

namespace NocInDan
{
    namespace
    {
        constexpr const char* g_kGroundAssetName = "NocInDan.WorldGround";
        constexpr float       g_kSlabHeight      = 0.5f;
        constexpr float       g_kWallHeight      = 4.0f;

        // Ocean (one merged mesh) and land chunks (one shared Ground-technique material), registered
        // as one asset and attached under p_parent.
        void buildGroundMeshes( Tomos::TScene& p_scene, Tomos::TVkGpu& p_gpu, Tomos::TSceneNode& p_parent, const WorldMap& p_map )
        {
            auto asset    = std::make_unique<Tomos::TGpuAsset>();
            asset->m_name = g_kGroundAssetName;

            struct Piece
            {
                std::string m_nodeName;
                uint32_t    m_material;
            };
            std::vector<Piece> pieces;

            auto loadAlbedo = [ & ]( const GroundTileDef& p_def ) -> const Tomos::TVkImage*
            { return p_scene.resources().loadImage( p_gpu, p_def.m_albedoPath, Tomos::TTexAddr::Repeat, /*p_required=*/false ); };
            auto addMaterial = [ & ]( const Tomos::TVkMaterialDesc& p_desc )
            {
                asset->m_materials.push_back( p_gpu.createMaterial( p_desc ) );
                return static_cast<uint32_t>( asset->m_materials.size() - 1 );
            };

            p_gpu.beginUploadBatch();

            if ( auto oceanMesh = makeOceanMesh( p_gpu, p_map ) )
            {
                const GroundTileDef&   ocean  = *findGroundTile( GroundTileId::Ocean );
                const Tomos::TVkImage* albedo = loadAlbedo( ocean );

                Tomos::TVkMaterialDesc desc{};
                desc.m_baseColorFactor = albedo != nullptr ? glm::vec4( 1.0f ) : glm::vec4( ocean.m_tint, 1.0f );
                desc.m_roughnessFactor = ocean.m_roughness;
                desc.m_emissionFactor  = albedo != nullptr ? glm::vec3( 0.0f ) : ocean.m_tint * 0.05f;
                desc.m_baseTexture     = albedo != nullptr ? albedo : &p_gpu.defaultTexture();

                asset->m_meshes.push_back( std::move( oceanMesh ) );
                pieces.push_back( { "ocean", addMaterial( desc ) } );
            }

            // Layer table indexed by GroundTileId; the shader blends between land layers per cell.
            std::array<GroundLayerGpu, Tomos::g_kMaxGroundLayers> layers{};
            static_assert( static_cast<size_t>( GroundTileId::Count ) <= Tomos::g_kMaxGroundLayers );
            for ( const GroundTileDef& def : builtinGroundTiles() )
            {
                const Tomos::TVkImage* albedo = loadAlbedo( def );
                GroundLayerGpu&        layer  = layers[ static_cast<size_t>( def.m_id ) ];
                layer.m_tint                  = albedo != nullptr ? glm::vec4( 1.0f ) : glm::vec4( def.m_tint, 1.0f );
                layer.m_texSlot               = p_gpu.renderer()->textureSlot( albedo );
                layer.m_uvScale               = def.m_uvScale;
                layer.m_roughness             = def.m_roughness;
                layer.m_blend                 = isLandGround( def.m_id ) ? 1u : 0u;
            }

            const std::vector<std::byte> groundData = buildGroundShaderData( p_map, layers );
            Tomos::TVkMaterialDesc       landDesc{};
            landDesc.m_technique        = Tomos::TMeshTechniqueId::Ground;
            landDesc.m_baseTexture      = &p_gpu.defaultTexture();
            landDesc.m_storageData      = groundData;
            const uint32_t landMaterial = addMaterial( landDesc );

            for ( int cy = 0; cy < p_map.chunkCountY(); ++cy )
            {
                for ( int cx = 0; cx < p_map.chunkCountX(); ++cx )
                {
                    auto mesh = makeLandChunkMesh( p_gpu, p_map, cx, cy );
                    if ( mesh == nullptr ) continue;
                    asset->m_meshes.push_back( std::move( mesh ) );
                    pieces.push_back( { "land " + std::to_string( cx ) + "," + std::to_string( cy ), landMaterial } );
                }
            }

            p_gpu.endUploadBatch();

            const Tomos::TGpuAsset* registered = Tomos::TApplication::get().assetSystem().registerAsset( asset, &p_gpu, /*p_replace=*/true );
            if ( registered == nullptr )
            {
                TLOG_ERROR() << "[WorldGeometry] failed to register ground meshes";
                return;
            }

            for ( size_t i = 0; i < pieces.size(); ++i )
            {
                Tomos::TSceneNode& patch = p_scene.createNode( pieces[ i ].m_nodeName );
                p_parent.addChild( &patch );
                // flat ground receives shadows but casts none
                patch.emplaceComponent<Tomos::TMeshComponent>( registered->mesh( static_cast<uint32_t>( i ) ), registered->material( pieces[ i ].m_material ),
                                                               /*p_castShadow=*/false );
            }
        }
    }  // namespace

    void buildWorldGeometry( Tomos::TScene& p_scene, Tomos::TVkGpu& p_gpu, Tomos::TSceneNode& p_parent, const WorldMap& p_map )
    {
        Tomos::TSceneNode& slab = p_scene.createNode( "Ground" );
        p_parent.addChild( &slab );
        makeStaticBox( slab, { 0.0f, -g_kSlabHeight * 0.5f, 0.0f }, { p_map.worldWidth(), g_kSlabHeight, p_map.worldDepth() } );

        Tomos::TSceneNode& meshes = p_scene.createNode( "GroundChunks" );
        p_parent.addChild( &meshes );
        buildGroundMeshes( p_scene, p_gpu, meshes, p_map );

        // Invisible walls along impassable edges: solid, sliding coastlines.
        Tomos::TSceneNode& walls = p_scene.createNode( "CoastWalls" );
        p_parent.addChild( &walls );
        for ( const WallBox& box : buildCoastWalls( p_map ) )
        {
            Tomos::TSceneNode& wall = p_scene.createNode( "Wall" );
            walls.addChild( &wall );
            makeStaticBox( wall, { box.m_center.x, 0.5f * g_kWallHeight, box.m_center.y }, { box.m_size.x, g_kWallHeight, box.m_size.y } );
        }
    }

}  // namespace NocInDan

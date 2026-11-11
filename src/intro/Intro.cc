#include "Intro.hh"

#include <GLFW/glfw3.h>
#include <memory>

#include "../scenes/InGameScene.hh"
#include "Tomos/core/app/TApplication.hh"
#include "Tomos/core/input/TInput.hh"
#include "Tomos/core/scene/TSceneManager.hh"
#include "Tomos/core/scene/TSceneResourceBag.hh"
#include "Tomos/gpu/TBillboardMode.hh"
#include "Tomos/gpu/TSpriteAlphaMode.hh"
#include "Tomos/gpu/vulkan/TVkGpu.hh"
#include "Tomos/systems/camera/TCameraComponent.hh"
#include "Tomos/systems/script/TScript.hh"
#include "Tomos/systems/script/TScriptComponent.hh"
#include "Tomos/systems/sprite/TSpriteComponent.hh"
#include "Tomos/util/logger/TLogger.hh"

namespace NocInDan
{
    namespace
    {
        constexpr const char* g_kBannerPath = "assets/banner.png";
        constexpr float       g_kDuration   = 6.0f;
        constexpr float       g_kFade       = 0.8f;

        class IntroScript : public Tomos::TScript
        {
        public:
            explicit IntroScript( Tomos::TSpriteComponent* p_sprite ) : m_sprite( p_sprite ) {}

            void onAttach() override { setAlpha( 0.0f ); }

            void update( float p_dt ) override
            {
                if ( m_done ) return;

                if ( Tomos::TInput::keyPressed( GLFW_KEY_SPACE, m_space ) || Tomos::TInput::keyPressed( GLFW_KEY_ESCAPE, m_esc ) )
                {
                    finish();
                    return;
                }

                m_t += p_dt;

                float alpha = 1.0f;
                if ( m_t < g_kFade )
                    alpha = m_t / g_kFade;
                else if ( m_t > g_kDuration - g_kFade )
                    alpha = glm::clamp( ( g_kDuration - m_t ) / g_kFade, 0.0f, 1.0f );

                setAlpha( alpha );
                if ( m_t >= g_kDuration ) finish();
            }

        private:
            void setAlpha( float p_alpha )
            {
                if ( m_sprite != nullptr ) m_sprite->m_color.a = p_alpha;
            }

            void finish()
            {
                if ( m_done ) return;
                m_done = true;
                Tomos::TApplication::get().sceneManager().queueScene( [] { return std::make_shared<InGameScene>(); } );
            }

            Tomos::TSpriteComponent* m_sprite = nullptr;
            float                    m_t      = 0.0f;
            bool                     m_done   = false;
            bool                     m_space  = false;
            bool                     m_esc    = false;
        };
    }  // namespace

    void spawnIntro( Tomos::TScene& p_scene, Tomos::TVkGpu& p_gpu )
    {
        Tomos::TVkImage* banner = p_scene.resources().loadImage( p_gpu, g_kBannerPath );
        if ( banner == nullptr || banner == &p_gpu.missingTexture() )
        {
            TLOG_ERROR() << "[Intro] failed to load " << g_kBannerPath;
            return;
        }

        auto& node = p_scene.createNode( "IntroBanner" );
        p_scene.addChild( &node );
        node.m_transform.setTranslation( glm::vec3( 0.0f, 0.0f, -3.2f ) );

        auto& sprite       = node.emplaceComponent<Tomos::TSpriteComponent>( banner );
        sprite.m_size      = { 3.0f, 1.0f };
        sprite.m_color     = { 1.0f, 1.0f, 1.0f, 0.0f };
        sprite.m_mode      = Tomos::TBillboardMode::Spherical;
        sprite.m_alphaMode = Tomos::TSpriteAlphaMode::Blend;
        sprite.m_emissive  = 1.0f;  // the intro scene has no lights

        auto& camNode = p_scene.createNode( "IntroCamera" );
        p_scene.addChild( &camNode );
        auto& cam    = camNode.emplaceComponent<Tomos::TCameraComponent>();
        cam.m_active = true;
        cam.m_fov    = glm::radians( 40.0f );

        auto& director = p_scene.createNode( "IntroDirector" );
        p_scene.addChild( &director );
        director.addComponent( Tomos::TScriptComponent::make<IntroScript>( &sprite ) );
    }
}  // namespace NocInDan

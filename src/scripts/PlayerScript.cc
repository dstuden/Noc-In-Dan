#include "PlayerScript.hh"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "../components/TiltedCameraComponent.hh"
#include "../scenes/InGameScene.hh"
#include "Tomos/core/app/TApplication.hh"
#include "Tomos/core/scene/TSceneNode.hh"
#include "Tomos/systems/physics/TRigidBodyComponent.hh"
#include "Tomos/ui/TImGuiBackend.hh"
#include "Tomos/util/memory/TLevelStore.hh"

namespace NocInDan
{
    PlayerScript::PlayerScript( Tomos::TNodeHandle p_camera ) : m_camera( p_camera ) {}

    void PlayerScript::bindActions()
    {
        if ( m_actionsBound ) return;
        m_actionsBound = true;
        m_actions.bindKeyAxis( "MoveX", GLFW_KEY_A, GLFW_KEY_D );
        m_actions.bindKeyAxis( "MoveZ", GLFW_KEY_S, GLFW_KEY_W );
        m_actions.bindKey( "CamRotateLeft", GLFW_KEY_Q );
        m_actions.bindKey( "CamRotateRight", GLFW_KEY_E );
        m_actions.bindKey( "CamZoomIn", GLFW_KEY_EQUAL );
        m_actions.bindKey( "CamZoomIn", GLFW_KEY_KP_ADD );
        m_actions.bindKey( "CamZoomOut", GLFW_KEY_MINUS );
        m_actions.bindKey( "CamZoomOut", GLFW_KEY_KP_SUBTRACT );
    }

    Tomos::TSceneNode* PlayerScript::cameraNode()
    {
        Tomos::TLevelStore* store = node().store();
        if ( Tomos::TSceneNode* cam = store != nullptr ? store->getNode( m_camera ) : nullptr ) return cam;

        Tomos::TSceneNode* cam = node().root().findByName( "Camera" );
        m_camera               = cam != nullptr ? cam->handle() : Tomos::TNodeHandle{};
        return cam;
    }

    void PlayerScript::onAttach()
    {
        bindActions();
        // saves may hold fewer slots than the default
        m_survival.m_inventory.ensureCapacity( g_kInventorySlots );

        if ( Tomos::TSceneNode* camNode = cameraNode() )
            if ( auto* cam = camNode->findComponent<TiltedCameraComponent>() ) cam->snap( *camNode );

        m_scene = InGameScene::of( node() );
        if ( m_scene != nullptr ) m_scene->m_player = this;
    }

    void PlayerScript::onDetach()
    {
        if ( m_scene != nullptr && m_scene->m_player == this ) m_scene->m_player = nullptr;
    }

    void PlayerScript::update( float /*p_dt*/ )
    {
        if ( m_body == nullptr ) m_body = node().findComponent<Tomos::TRigidBodyComponent>();
        if ( m_body == nullptr ) return;
        Tomos::TSceneNode*     camNode = cameraNode();
        TiltedCameraComponent* cam     = camNode != nullptr ? camNode->findComponent<TiltedCameraComponent>() : nullptr;
        if ( cam == nullptr ) return;

        m_locomotor.setWorldMap( m_scene != nullptr ? m_scene->worldMap() : nullptr );

        GLFWwindow*                  window = Tomos::TApplication::get().window().getNativeWindow();
        const Tomos::TInput::BlockFn block  = [] { return Tomos::TImGuiBackend::ioBlocksGameInput(); };

        if ( cam->m_controllable )
        {
            if ( m_actions.pressed( "CamRotateLeft", m_rotL, window, block ) ) cam->rotate( +1.0f );
            if ( m_actions.pressed( "CamRotateRight", m_rotR, window, block ) ) cam->rotate( -1.0f );
            if ( m_actions.pressed( "CamZoomIn", m_zoomIn, window, block ) ) cam->zoom( +1.0f );
            if ( m_actions.pressed( "CamZoomOut", m_zoomOut, window, block ) ) cam->zoom( -1.0f );
        }

        const float moveX = m_actions.value( "MoveX", window, block );
        const float moveZ = m_actions.value( "MoveZ", window, block );

        const glm::vec2 wish = cam->groundRight() * moveX + cam->groundForward() * moveZ;

        m_locomotor.m_runSpeed = m_moveSpeed;
        const glm::vec3  pos   = glm::vec3( node().worldMatrix()[ 3 ] );
        const MoveResult move  = m_locomotor.update( pos, wish );
        applyMove( *m_body, move );
    }

    void PlayerScript::lateUpdate( float p_dt )
    {
        if ( Tomos::TSceneNode* camNode = cameraNode() )
            if ( auto* cam = camNode->findComponent<TiltedCameraComponent>() ) cam->follow( *camNode, p_dt );
    }
}  // namespace NocInDan

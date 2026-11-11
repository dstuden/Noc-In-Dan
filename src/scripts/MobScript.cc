#include "MobScript.hh"

#include <cmath>
#include <glm/gtc/constants.hpp>

#include "../scenes/InGameScene.hh"
#include "Tomos/core/scene/TSceneNode.hh"
#include "Tomos/systems/physics/TRigidBodyComponent.hh"
#include "Tomos/systems/spriterig/TSpriteRigComponent.hh"
#include "Tomos/util/random/TRandom.hh"

namespace NocInDan
{
    void MobScript::onAttach()
    {
        m_scene       = InGameScene::of( node() );
        m_body        = node().findComponent<Tomos::TRigidBodyComponent>();
        m_rig         = node().findComponent<Tomos::TSpriteRigComponent>();
        m_actionTimer = Tomos::TRandom::global().nextFloat( 0.0f, 1.5f );
    }

    void MobScript::pickNextAction()
    {
        auto& rng     = Tomos::TRandom::global();
        m_actionTimer = rng.nextFloat( 1.5f, 4.0f );
        if ( rng.nextBool( 0.4f ) )
        {
            m_wish = glm::vec2( 0.0f );
            return;
        }
        const float a = rng.nextFloat( 0.0f, glm::two_pi<float>() );
        m_wish        = glm::vec2( std::cos( a ), std::sin( a ) );
    }

    void MobScript::update( float p_dt )
    {
        if ( m_body == nullptr ) return;

        m_actionTimer -= p_dt;
        if ( m_actionTimer <= 0.0f ) pickNextAction();

        m_locomotor.setWorldMap( m_scene != nullptr ? m_scene->worldMap() : nullptr );
        m_locomotor.m_runSpeed = m_walkSpeed;

        const glm::vec3  pos  = glm::vec3( node().worldMatrix()[ 3 ] );
        const MoveResult move = m_locomotor.update( pos, m_wish );
        applyMove( *m_body, move );

        if ( m_rig != nullptr ) m_rig->play( move.m_state == MoveState::Running ? "walk" : "idle" );
    }
}  // namespace NocInDan

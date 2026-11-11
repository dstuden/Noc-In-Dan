#include "TiltedCameraComponent.hh"

#include <cmath>
#include <glm/gtc/quaternion.hpp>

#include "Tomos/util/memory/TLevelStore.hh"

namespace NocInDan
{
    namespace
    {
        // Frame-rate independent exponential approach; gain <= 0 snaps.
        template<typename T>
        T damp( const T& p_current, const T& p_target, float p_gain, float p_dt )
        {
            if ( p_gain <= 0.0f ) return p_target;
            const float t = 1.0f - std::exp( -p_gain * p_dt );
            return p_current + ( p_target - p_current ) * t;
        }
    }  // namespace

    TiltedCameraComponent::TiltedCameraComponent()
    {
        m_active     = true;
        m_projection = Tomos::TProjection::Perspective;
        m_fov        = glm::radians( 35.0f );
        m_near       = 0.5f;
        m_far        = 2500.0f;
    }

    void TiltedCameraComponent::setTarget( Tomos::TSceneNode* p_target ) { m_target = p_target != nullptr ? p_target->handle() : Tomos::TNodeHandle{}; }

    void TiltedCameraComponent::zoom( float p_steps ) { m_distanceTarget = glm::clamp( m_distanceTarget - p_steps * m_zoomStep, m_minDist, m_maxDist ); }

    void TiltedCameraComponent::rotate( float p_steps ) { m_headingTarget += p_steps * m_rotateStep; }

    Tomos::TSceneNode* TiltedCameraComponent::resolveTarget( Tomos::TSceneNode& p_cameraNode )
    {
        Tomos::TLevelStore* store = p_cameraNode.store();
        if ( store == nullptr || !m_target ) return nullptr;
        return store->getNode( m_target );
    }

    void TiltedCameraComponent::snap( Tomos::TSceneNode& p_cameraNode )
    {
        m_distance = m_distanceTarget;
        m_heading  = m_headingTarget;
        if ( Tomos::TSceneNode* target = resolveTarget( p_cameraNode ) )
        {
            m_focus    = glm::vec3( target->worldMatrix()[ 3 ] ) + m_targetOffset;
            m_hasFocus = true;
        }
    }

    float TiltedCameraComponent::pitchForDistance( float p_distance ) const
    {
        const float span = m_maxDist - m_minDist;
        const float t    = span > 1e-4f ? glm::clamp( ( p_distance - m_minDist ) / span, 0.0f, 1.0f ) : 0.0f;
        return glm::mix( m_minPitch, m_maxPitch, t );
    }

    glm::vec3 TiltedCameraComponent::eyeFromFocus( const glm::vec3& p_focus, float p_heading, float p_pitch, float p_distance ) const
    {
        const float cp = std::cos( p_pitch );
        const float sp = std::sin( p_pitch );
        const float ch = std::cos( p_heading );
        const float sh = std::sin( p_heading );
        return p_focus + glm::vec3( p_distance * cp * ch, p_distance * sp, p_distance * cp * sh );
    }

    void TiltedCameraComponent::applyPose( Tomos::TSceneNode& p_cameraNode, const glm::vec3& p_eye, const glm::vec3& p_focus )
    {
        const glm::vec3 toFocus = p_focus - p_eye;
        const float     lenSq   = glm::dot( toFocus, toFocus );
        if ( lenSq < 1e-8f ) return;

        const glm::vec3 forward = toFocus / std::sqrt( lenSq );
        const glm::vec3 worldUp( 0.0f, 1.0f, 0.0f );
        const glm::vec3 up = std::abs( glm::dot( forward, worldUp ) ) > 0.999f ? glm::vec3( 0.0f, 0.0f, 1.0f ) : worldUp;

        p_cameraNode.m_transform.setLocalTRS( p_eye, glm::quatLookAt( forward, up ), glm::vec3( 1.0f ) );
    }

    void TiltedCameraComponent::follow( Tomos::TSceneNode& p_cameraNode, float p_dt, const glm::vec3* p_focusOverride )
    {
        glm::vec3 desiredFocus = m_focus;
        if ( p_focusOverride != nullptr )
            desiredFocus = *p_focusOverride + m_targetOffset;
        else if ( Tomos::TSceneNode* target = resolveTarget( p_cameraNode ) )
            desiredFocus = glm::vec3( target->worldMatrix()[ 3 ] ) + m_targetOffset;

        if ( !m_hasFocus )
        {
            m_focus    = desiredFocus;
            m_hasFocus = true;
        }
        else
        {
            m_focus = damp( m_focus, desiredFocus, m_panGain, p_dt );
        }

        m_heading  = damp( m_heading, m_headingTarget, m_headingGain, p_dt );
        m_distance = damp( m_distance, m_distanceTarget, m_distanceGain, p_dt );
        m_distance = glm::clamp( m_distance, m_minDist, m_maxDist );

        const float     pitch = pitchForDistance( m_distance );
        const glm::vec3 eye   = eyeFromFocus( m_focus, m_heading, pitch, m_distance );
        applyPose( p_cameraNode, eye, m_focus );
    }
}  // namespace NocInDan

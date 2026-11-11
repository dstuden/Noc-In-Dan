#pragma once

#include <cmath>
#include <glm/glm.hpp>

#include "Tomos/core/scene/TSceneNode.hh"
#include "Tomos/systems/camera/TCameraComponent.hh"
#include "Tomos/util/memory/THandle.hh"
#include "Tomos/util/reflect/TReflectAttr.hh"

namespace NocInDan
{
    class TOMOS_ANN( Tomos::Reflect::ComponentMeta{ "tiltedCamera", "Tilted Camera" } ) TiltedCameraComponent : public Tomos::TCameraComponent
    {
    public:
        TiltedCameraComponent();

        Tomos::TNodeHandle m_target{};

        glm::vec3 m_targetOffset{ 0.0f, 1.5f, 0.0f };

        float m_distance                                           = 30.0f;
        TOMOS_ANN( Tomos::Reflect::Skip{} ) float m_distanceTarget = 30.0f;
        float m_minDist                                            = 15.0f;
        float m_maxDist                                            = 200.0f;
        float m_zoomStep                                           = 4.0f;

        TOMOS_ANN( Tomos::Reflect::Degrees{} ) float m_heading                                           = glm::radians( 45.0f );
        TOMOS_ANN( Tomos::Reflect::Skip{} ) TOMOS_ANN( Tomos::Reflect::Degrees{} ) float m_headingTarget = glm::radians( 45.0f );
        TOMOS_ANN( Tomos::Reflect::Degrees{} ) float m_rotateStep                                        = glm::radians( 45.0f );

        TOMOS_ANN( Tomos::Reflect::Degrees{} ) float m_minPitch = glm::radians( 30.0f );
        TOMOS_ANN( Tomos::Reflect::Degrees{} ) float m_maxPitch = glm::radians( 60.0f );

        float m_panGain      = 4.0f;
        float m_headingGain  = 20.0f;
        float m_distanceGain = 1.0f;

        bool m_controllable = true;

        void setTarget( Tomos::TSceneNode* p_target );
        void zoom( float p_steps );
        void rotate( float p_steps );
        void snap( Tomos::TSceneNode& p_cameraNode );

        void follow( Tomos::TSceneNode& p_cameraNode, float p_dt, const glm::vec3* p_focusOverride = nullptr );

        // Screen-up and screen-right projected onto the XZ ground plane (as x, z).
        [[nodiscard]] glm::vec2 groundForward() const { return { -std::cos( m_heading ), -std::sin( m_heading ) }; }
        [[nodiscard]] glm::vec2 groundRight() const { return { std::sin( m_heading ), -std::cos( m_heading ) }; }

    private:
        [[nodiscard]] Tomos::TSceneNode* resolveTarget( Tomos::TSceneNode& p_cameraNode );
        [[nodiscard]] float              pitchForDistance( float p_distance ) const;
        [[nodiscard]] glm::vec3          eyeFromFocus( const glm::vec3& p_focus, float p_heading, float p_pitch, float p_distance ) const;
        void                             applyPose( Tomos::TSceneNode& p_cameraNode, const glm::vec3& p_eye, const glm::vec3& p_focus );

        glm::vec3 m_focus{ 0.0f };
        bool      m_hasFocus = false;
    };
}  // namespace NocInDan

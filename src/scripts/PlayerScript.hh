#pragma once

#include "../movement/LocomotorController.hh"
#include "../utils/Survival.hh"
#include "Tomos/core/input/TActionMap.hh"
#include "Tomos/systems/script/TScript.hh"
#include "Tomos/util/memory/THandle.hh"

namespace Tomos
{
    class TRigidBodyComponent;
}
namespace NocInDan
{
    class InGameScene;
    class TiltedCameraComponent;

    class PlayerScript : public Tomos::TScript
    {
    public:
        PlayerScript() = default;
        explicit PlayerScript( Tomos::TNodeHandle p_camera );

        void onAttach() override;
        void onDetach() override;
        void update( float p_dt ) override;
        // camera follow, after physics interpolation
        void lateUpdate( float p_dt ) override;

        Tomos::TNodeHandle m_camera{};
        float              m_moveSpeed = 8.0f;
        Survival           m_survival;

    private:
        void                             bindActions();
        [[nodiscard]] Tomos::TSceneNode* cameraNode();

        InGameScene*        m_scene = nullptr;
        LocomotorController m_locomotor;

        Tomos::TActionMap m_actions;
        bool              m_actionsBound = false;

        Tomos::TRigidBodyComponent* m_body = nullptr;

        bool m_rotL    = false;
        bool m_rotR    = false;
        bool m_zoomIn  = false;
        bool m_zoomOut = false;
    };
}  // namespace NocInDan

#pragma once

#include "../movement/LocomotorController.hh"
#include "Tomos/systems/script/TScript.hh"

namespace Tomos
{
    class TRigidBodyComponent;
    class TSpriteRigComponent;
}  // namespace Tomos

namespace NocInDan
{
    class InGameScene;

    // Wanders: alternates random walks and pauses. The rig turns to the movement on its own.
    class MobScript : public Tomos::TScript
    {
    public:
        void onAttach() override;
        void update( float p_dt ) override;

        float m_walkSpeed = 2.0f;

    private:
        void pickNextAction();

        InGameScene*        m_scene = nullptr;
        LocomotorController m_locomotor;
        glm::vec2           m_wish{ 0.0f };
        float               m_actionTimer = 0.0f;

        Tomos::TRigidBodyComponent* m_body = nullptr;
        Tomos::TSpriteRigComponent* m_rig  = nullptr;
    };
}  // namespace NocInDan

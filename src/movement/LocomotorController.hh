#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <string>
#include <string_view>
#include <vector>

#include "SpeedModifierList.hh"

namespace Tomos
{
    class TRigidBodyComponent;
}

namespace NocInDan
{
    struct WorldMap;

    enum class MoveState : uint8_t
    {
        Idle,
        Running,
        Busy,
        Frozen,
    };

    struct MoveResult
    {
        glm::vec2 m_velocity{ 0.0f };
        MoveState m_state = MoveState::Idle;
    };

    // Planar velocity from speed, modifiers and the ground under the mover. Coast walls keep movers
    // on land; this only decides how fast they go. Apply with applyMove().
    class LocomotorController
    {
    public:
        float m_runSpeed = 8.0f;

        void setWorldMap( const WorldMap* p_map ) { m_map = p_map; }

        SpeedModifierList&                     speedModifiers() { return m_speedMods; }
        [[nodiscard]] const SpeedModifierList& speedModifiers() const { return m_speedMods; }

        void               addMovementBlocker( std::string_view p_source );
        void               removeMovementBlocker( std::string_view p_source );
        [[nodiscard]] bool hasMovementBlocker() const { return !m_blockers.empty(); }

        void               setFrozen( bool p_frozen ) { m_frozen = p_frozen; }
        [[nodiscard]] bool frozen() const { return m_frozen; }

        // p_wishDir: desired XZ direction; clamped to length 1 (analog magnitude scales speed).
        MoveResult update( const glm::vec3& p_pos, glm::vec2 p_wishDir );

        [[nodiscard]] float currentSpeed( const glm::vec3& p_pos ) const;

        [[nodiscard]] MoveState state() const { return m_state; }

    private:
        const WorldMap*          m_map = nullptr;
        SpeedModifierList        m_speedMods;
        std::vector<std::string> m_blockers;
        bool                     m_frozen = false;
        MoveState                m_state  = MoveState::Idle;
    };

    // Set the planar velocity of p_body (gravity keeps the vertical one).
    void applyMove( Tomos::TRigidBodyComponent& p_body, const MoveResult& p_move );

}  // namespace NocInDan

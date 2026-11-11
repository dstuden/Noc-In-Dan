#include "LocomotorController.hh"

#include <algorithm>
#include <cmath>

#include "../worldgen/WorldMap.hh"
#include "Tomos/systems/physics/TRigidBodyComponent.hh"

namespace NocInDan
{
    namespace
    {
        constexpr float g_kEpsilonSq = 1e-8f;
    }

    void LocomotorController::addMovementBlocker( std::string_view p_source )
    {
        if ( std::find( m_blockers.begin(), m_blockers.end(), p_source ) == m_blockers.end() ) m_blockers.emplace_back( p_source );
    }

    void LocomotorController::removeMovementBlocker( std::string_view p_source )
    {
        std::erase_if( m_blockers, [ & ]( const std::string& s ) { return s == p_source; } );
    }

    float LocomotorController::currentSpeed( const glm::vec3& p_pos ) const
    {
        float ground = 1.0f;
        if ( m_map != nullptr )
        {
            const TileMovementRules& rules = m_map->rulesAtPoint( p_pos.x, p_pos.z );
            // impassable ground has no multiplier: a mover that ends up there can still walk off
            if ( rules.m_passable ) ground = rules.m_speedMultiplier;
        }
        return std::max( 0.0f, m_runSpeed ) * m_speedMods.get() * ground;
    }

    MoveResult LocomotorController::update( const glm::vec3& p_pos, glm::vec2 p_wishDir )
    {
        MoveResult out;

        const float wishLenSq = glm::dot( p_wishDir, p_wishDir );
        if ( m_frozen )
            out.m_state = MoveState::Frozen;
        else if ( !m_blockers.empty() )
            out.m_state = MoveState::Busy;
        else if ( wishLenSq >= g_kEpsilonSq )
        {
            if ( wishLenSq > 1.0f ) p_wishDir /= std::sqrt( wishLenSq );
            out.m_velocity = p_wishDir * currentSpeed( p_pos );
            out.m_state    = MoveState::Running;
        }

        m_state = out.m_state;
        return out;
    }

    void applyMove( Tomos::TRigidBodyComponent& p_body, const MoveResult& p_move )
    {
        p_body.m_linearVelocity.x = p_move.m_velocity.x;
        p_body.m_linearVelocity.z = p_move.m_velocity.y;
    }

}  // namespace NocInDan

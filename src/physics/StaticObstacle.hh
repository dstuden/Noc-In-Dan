#pragma once

#include <glm/glm.hpp>

#include "Tomos/core/scene/TSceneNode.hh"
#include "Tomos/systems/physics/TColliderComponent.hh"
#include "Tomos/systems/physics/TPhysicsLayers.hh"
#include "Tomos/systems/physics/TRigidBodyComponent.hh"

namespace NocInDan
{

    inline void configureStaticObstacle( Tomos::TColliderComponent& p_col )
    {
        p_col.m_layer = Tomos::TPhysicsLayer::g_static;
        p_col.m_mask  = Tomos::TPhysicsLayer::g_player | Tomos::TPhysicsLayer::g_dynamic;
    }

    inline void makeStaticBox( Tomos::TSceneNode& p_node, const glm::vec3& p_center, const glm::vec3& p_size )
    {
        p_node.m_transform.setTranslation( p_center );
        p_node.m_transform.setScale( p_size );

        auto& col         = p_node.emplaceComponent<Tomos::TColliderComponent>();
        col.m_shape       = Tomos::TColliderShape::Box;
        col.m_halfExtents = { 0.5f, 0.5f, 0.5f };
        configureStaticObstacle( col );
    }

    inline void makeDynamicSphere( Tomos::TSceneNode& p_node, float p_radius, uint32_t p_layer, uint32_t p_mask )
    {
        auto& body = p_node.emplaceComponent<Tomos::TRigidBodyComponent>();
        body.setMass( 1.0f );
        body.m_linearDamping = 0.0f;
        body.m_restitution   = 0.0f;

        auto& col    = p_node.emplaceComponent<Tomos::TColliderComponent>();
        col.m_shape  = Tomos::TColliderShape::Sphere;
        col.m_radius = p_radius;
        col.m_layer  = p_layer;
        col.m_mask   = p_mask;
    }

}  // namespace NocInDan

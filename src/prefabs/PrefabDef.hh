#pragma once

#include <glm/glm.hpp>
#include <string>

#include "Tomos/gpu/TSpriteShadowMode.hh"

namespace NocInDan
{
    // One JSON file in assets/prefabs (id = filename stem). Drawn as an upright sprite standing on
    // its spawn point. Prefabs with a collider become a node each; the rest are decoration and are
    // batched per world chunk (see buildSpawnChunk).
    struct PrefabDef
    {
        std::string m_id;

        glm::vec2 m_size{ 1.0f, 1.0f };
        glm::vec4 m_color{ 1.0f, 1.0f, 1.0f, 1.0f };

        std::string m_texturePath;

        // Upright cylinder centred on the spawn point (half of it below ground).
        bool  m_hasCollider        = false;
        float m_colliderRadius     = 0.35f;
        float m_colliderHalfHeight = 1.0f;

        // Shadow style; JSON uses the lowercase name: "upright" (structures, trees),
        // "contact" (mobs, items), "asrendered" or "none". Contact radius is a fraction
        // of the width (0 = no contact disc).
        Tomos::TSpriteShadowMode m_shadow              = Tomos::TSpriteShadowMode::Upright;
        float                    m_contactShadowRadius = 0.25f;
    };
}  // namespace NocInDan

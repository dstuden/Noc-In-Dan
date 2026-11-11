#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "GroundTileId.hh"

namespace NocInDan
{

    // One def per GroundTileId. Empty/missing albedo -> solid tint.
    struct GroundTileDef
    {
        GroundTileId      m_id = GroundTileId::None;
        std::string       m_name;
        std::string       m_albedoPath;
        glm::vec3         m_tint      = { 1.0f, 1.0f, 1.0f };
        float             m_uvScale   = 1.0f;  // texture repeats per cell
        float             m_roughness = 0.85f;
        TileMovementRules m_movement;
    };

    const std::vector<GroundTileDef>& builtinGroundTiles();

    const GroundTileDef* findGroundTile( GroundTileId p_id );

}  // namespace NocInDan

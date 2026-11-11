#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "WorldMap.hh"

namespace NocInDan
{

    // Axis-aligned XZ footprint of one invisible wall box (world space).
    struct WallBox
    {
        glm::vec2 m_center{ 0.0f };
        glm::vec2 m_size{ 0.0f };
    };

    // Wall boxes over impassable cells touching land (8-neighbourhood), plus map-edge borders.
    [[nodiscard]] std::vector<WallBox> buildCoastWalls( const WorldMap& p_world );

}  // namespace NocInDan

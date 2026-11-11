#pragma once

#include <cstdint>

namespace NocInDan
{

    // Ground type per cell; a biome paints a weighted mix of these.
    enum class GroundTileId : uint8_t
    {
        None   = 0,
        Ocean  = 1,
        Grass  = 2,
        Forest = 3,
        Rocky  = 4,
        Marsh  = 5,
        Sand   = 6,
        Snow   = 7,
        Road   = 8,
        Count
    };

    constexpr bool isLandGround( GroundTileId p_id ) { return p_id != GroundTileId::None && p_id != GroundTileId::Ocean; }

    struct TileMovementRules
    {
        bool  m_passable        = true;
        float m_speedMultiplier = 1.0f;
    };

    // unknown ids are impassable
    const TileMovementRules& movementRules( GroundTileId p_id );

}  // namespace NocInDan

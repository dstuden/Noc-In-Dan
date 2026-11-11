#pragma once

#include <cstdint>

namespace NocInDan
{

    enum class BiomeId : uint8_t
    {
        None   = 0,
        Desert = 1,
        Forest = 2,
        Plains = 3,
        Swamp  = 4,
        Tundra = 5,
        Ocean  = 6,
        Count
    };

    constexpr bool isLandBiome( BiomeId p_id ) { return p_id != BiomeId::None && p_id != BiomeId::Ocean && p_id != BiomeId::Count; }

}  // namespace NocInDan

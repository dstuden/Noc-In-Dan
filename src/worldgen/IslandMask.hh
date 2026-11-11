#pragma once

#include <cstdint>
#include <vector>

namespace NocInDan
{

    struct IslandMaskParams
    {
        int   m_oceanMargin       = 12;
        float m_islandRadiusScale = 0.78f;
        float m_coastNoiseAmp     = 0.22f;
        float m_coastNoiseFreq    = 3.2f;
    };

    [[nodiscard]] std::vector<uint8_t> fillIslandMask( int p_width, int p_height, uint64_t p_seed, const IslandMaskParams& p_params );

}  // namespace NocInDan

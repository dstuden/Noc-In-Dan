#include "IslandMask.hh"

#include <algorithm>
#include <cmath>

#include "Noise.hh"

namespace NocInDan
{
    namespace
    {
        using Noise::hash01;
        using Noise::valueNoise2;

        float seededUnit( uint64_t p_seed, uint32_t p_salt )
        {
            return hash01( static_cast<int>( p_salt ), static_cast<int>( p_seed >> 32 ), static_cast<uint32_t>( p_seed ) ^ p_salt );
        }
    }  // namespace

    std::vector<uint8_t> fillIslandMask( int p_width, int p_height, uint64_t p_seed, const IslandMaskParams& p_params )
    {
        if ( p_width <= 0 || p_height <= 0 ) return {};

        const int margin = std::clamp( p_params.m_oceanMargin, 0, std::min( p_width, p_height ) / 2 );
        const int innerW = p_width - 2 * margin;
        const int innerH = p_height - 2 * margin;
        if ( innerW <= 0 || innerH <= 0 ) return std::vector<uint8_t>( static_cast<size_t>( p_width ) * static_cast<size_t>( p_height ), 0 );

        const float radiusScale = std::clamp( p_params.m_islandRadiusScale, 0.15f, 1.25f );
        const float noiseAmp    = std::max( 0.0f, p_params.m_coastNoiseAmp );
        const float noiseFreq   = std::max( 0.1f, p_params.m_coastNoiseFreq );

        const float aspectX = std::lerp( 0.72f, 1.18f, seededUnit( p_seed, 0xA11Cu ) );
        const float aspectY = std::lerp( 0.72f, 1.18f, seededUnit( p_seed, 0xB22Du ) );

        const float cx    = 0.5f * static_cast<float>( p_width - 1 );
        const float cy    = 0.5f * static_cast<float>( p_height - 1 );
        const float halfX = 0.5f * static_cast<float>( innerW ) * radiusScale * aspectX;
        const float halfY = 0.5f * static_cast<float>( innerH ) * radiusScale * aspectY;
        const float invX  = halfX > 1.0e-4f ? 1.0f / halfX : 0.0f;
        const float invY  = halfY > 1.0e-4f ? 1.0f / halfY : 0.0f;

        const uint32_t noiseSeed = Noise::foldSeed( p_seed, 0xC0A57u );

        std::vector<uint8_t> mask( static_cast<size_t>( p_width ) * static_cast<size_t>( p_height ), 0 );
        for ( int y = 0; y < p_height; ++y )
        {
            for ( int x = 0; x < p_width; ++x )
            {
                if ( x < margin || y < margin || x >= p_width - margin || y >= p_height - margin ) continue;

                const float nx = ( static_cast<float>( x ) - cx ) * invX;
                const float ny = ( static_cast<float>( y ) - cy ) * invY;
                float       r  = std::sqrt( nx * nx + ny * ny );

                if ( noiseAmp > 0.0f )
                {
                    const float n = valueNoise2( nx * noiseFreq + 17.3f, ny * noiseFreq + 9.1f, noiseSeed );
                    r += ( n * 2.0f - 1.0f ) * noiseAmp;
                }

                if ( r <= 1.0f ) mask[ static_cast<size_t>( y * p_width + x ) ] = 1;
            }
        }
        return mask;
    }

}  // namespace NocInDan

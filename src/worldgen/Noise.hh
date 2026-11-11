#pragma once

#include <cmath>
#include <cstdint>
#include <string_view>

namespace NocInDan::Noise
{

    inline uint32_t foldSeed( uint64_t p_seed, uint32_t p_salt ) { return static_cast<uint32_t>( p_seed ) ^ static_cast<uint32_t>( p_seed >> 32 ) ^ p_salt; }

    // Golden-ratio mix: independent seed per index from one base.
    inline uint32_t saltSeed( uint32_t p_seed, size_t p_index ) { return p_seed ^ ( static_cast<uint32_t>( p_index + 1 ) * 0x9E3779B9u ); }

    // FNV-1a.
    inline uint32_t hashString( std::string_view p_text )
    {
        uint32_t h = 2166136261u;
        for ( const char c : p_text ) h = ( h ^ static_cast<uint8_t>( c ) ) * 16777619u;
        return h;
    }

    inline uint32_t hash2( int p_x, int p_y, uint32_t p_seed )
    {
        uint32_t h = p_seed;
        h ^= static_cast<uint32_t>( p_x ) * 374761393u;
        h ^= static_cast<uint32_t>( p_y ) * 668265263u;
        h = ( h ^ ( h >> 13 ) ) * 1274126177u;
        return h ^ ( h >> 16 );
    }

    inline float hash01( int p_x, int p_y, uint32_t p_seed )
    {
        return static_cast<float>( hash2( p_x, p_y, p_seed ) & 0x00FFFFFFu ) / static_cast<float>( 0x00FFFFFFu );
    }

    inline float valueNoise2( float p_x, float p_y, uint32_t p_seed )
    {
        const int   x0 = static_cast<int>( std::floor( p_x ) );
        const int   y0 = static_cast<int>( std::floor( p_y ) );
        const float fx = p_x - static_cast<float>( x0 );
        const float fy = p_y - static_cast<float>( y0 );
        const float u  = fx * fx * ( 3.0f - 2.0f * fx );
        const float v  = fy * fy * ( 3.0f - 2.0f * fy );

        const float a = hash01( x0, y0, p_seed );
        const float b = hash01( x0 + 1, y0, p_seed );
        const float c = hash01( x0, y0 + 1, p_seed );
        const float d = hash01( x0 + 1, y0 + 1, p_seed );
        return std::lerp( std::lerp( a, b, u ), std::lerp( c, d, u ), v );
    }

}  // namespace NocInDan::Noise

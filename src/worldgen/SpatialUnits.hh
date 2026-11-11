#pragma once

#include <algorithm>
#include <compare>
#include <cstdlib>

namespace NocInDan
{

    struct Cells
    {
        int count = 0;

        constexpr Cells() = default;
        constexpr explicit Cells( int p_count ) : count( p_count ) {}
        constexpr auto operator<=>( const Cells& ) const = default;
    };

    struct WorldUnits
    {
        float value = 0.0f;

        constexpr WorldUnits() = default;
        constexpr explicit WorldUnits( float p_value ) : value( p_value ) {}
        constexpr auto operator<=>( const WorldUnits& ) const = default;
    };

    constexpr WorldUnits operator*( int p_cells, WorldUnits p_perCell ) { return WorldUnits{ static_cast<float>( p_cells ) * p_perCell.value }; }

    constexpr WorldUnits operator*( WorldUnits p_perCell, int p_cells ) { return p_cells * p_perCell; }

    inline Cells chebyshevCells( int p_ax, int p_ay, int p_bx, int p_by ) { return Cells{ std::max( std::abs( p_ax - p_bx ), std::abs( p_ay - p_by ) ) }; }

    inline int cellDistSq( int p_ax, int p_ay, int p_bx, int p_by )
    {
        const int dx = p_ax - p_bx;
        const int dy = p_ay - p_by;
        return dx * dx + dy * dy;
    }

    inline bool withinPlayerClear( int p_x, int p_y, int p_spawnX, int p_spawnY, Cells p_radius )
    {
        return chebyshevCells( p_x, p_y, p_spawnX, p_spawnY ) <= p_radius;
    }

}  // namespace NocInDan

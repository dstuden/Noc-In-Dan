#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace NocInDan
{
    // Inclusive cell rectangle.
    struct CellRect
    {
        int m_x0 = 0;
        int m_y0 = 0;
        int m_x1 = 0;
        int m_y1 = 0;
    };

    // Greedy rectangle cover of the cells where p_include(x, y) holds.
    template<typename Pred>
    [[nodiscard]] std::vector<CellRect> mergeCellRects( int p_width, int p_height, Pred&& p_include )
    {
        std::vector<CellRect> rects;
        if ( p_width <= 0 || p_height <= 0 ) return rects;

        std::vector<uint8_t> open( static_cast<size_t>( p_width ) * static_cast<size_t>( p_height ), 0 );
        auto                 at = [ & ]( int p_x, int p_y ) -> uint8_t&
        { return open[ static_cast<size_t>( p_y ) * static_cast<size_t>( p_width ) + static_cast<size_t>( p_x ) ]; };
        for ( int y = 0; y < p_height; ++y )
            for ( int x = 0; x < p_width; ++x ) at( x, y ) = p_include( x, y ) ? 1 : 0;

        for ( int y = 0; y < p_height; ++y )
        {
            for ( int x = 0; x < p_width; ++x )
            {
                if ( at( x, y ) == 0 ) continue;

                int x1 = x;
                while ( x1 + 1 < p_width && at( x1 + 1, y ) != 0 ) ++x1;

                int  y1       = y;
                bool rowFills = true;
                while ( rowFills && y1 + 1 < p_height )
                {
                    for ( int cx = x; cx <= x1 && rowFills; ++cx ) rowFills = at( cx, y1 + 1 ) != 0;
                    if ( rowFills ) ++y1;
                }

                for ( int cy = y; cy <= y1; ++cy )
                    for ( int cx = x; cx <= x1; ++cx ) at( cx, cy ) = 0;
                rects.push_back( { x, y, x1, y1 } );
            }
        }
        return rects;
    }
}  // namespace NocInDan

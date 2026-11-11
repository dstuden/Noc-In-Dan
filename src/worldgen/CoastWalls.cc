#include "CoastWalls.hh"

#include "CellRects.hh"

namespace NocInDan
{
    namespace
    {
        bool isWallCell( const WorldMap& p_w, int p_x, int p_y )
        {
            if ( p_w.passableAt( p_x, p_y ) ) return false;
            for ( int dy = -1; dy <= 1; ++dy )
                for ( int dx = -1; dx <= 1; ++dx )
                    if ( ( dx != 0 || dy != 0 ) && p_w.passableAt( p_x + dx, p_y + dy ) ) return true;
            return false;
        }
    }  // namespace

    std::vector<WallBox> buildCoastWalls( const WorldMap& p_world )
    {
        std::vector<WallBox> walls;

        const int   w = p_world.m_width;
        const int   h = p_world.m_height;
        const float s = p_world.m_cellSize.value;

        for ( const CellRect& r : mergeCellRects( w, h, [ & ]( int p_x, int p_y ) { return isWallCell( p_world, p_x, p_y ); } ) )
        {
            walls.push_back( { { 0.5f * ( p_world.cellWorldX( r.m_x0 ) + p_world.cellWorldX( r.m_x1 ) ),
                                 0.5f * ( p_world.cellWorldZ( r.m_y0 ) + p_world.cellWorldZ( r.m_y1 ) ) },
                               { static_cast<float>( r.m_x1 - r.m_x0 + 1 ) * s, static_cast<float>( r.m_y1 - r.m_y0 + 1 ) * s } } );
        }

        // Map border, only on sides where land reaches the grid edge (no ocean margin there).
        auto edgeHasLand = [ & ]( bool p_horizontal, int p_fixed )
        {
            const int n = p_horizontal ? w : h;
            for ( int i = 0; i < n; ++i )
                if ( p_horizontal ? p_world.passableAt( i, p_fixed ) : p_world.passableAt( p_fixed, i ) ) return true;
            return false;
        };
        const float mapW = p_world.worldWidth();
        const float mapD = p_world.worldDepth();
        if ( edgeHasLand( true, 0 ) ) walls.push_back( { { 0.0f, -0.5f * ( mapD + s ) }, { mapW + 2.0f * s, s } } );
        if ( edgeHasLand( true, h - 1 ) ) walls.push_back( { { 0.0f, 0.5f * ( mapD + s ) }, { mapW + 2.0f * s, s } } );
        if ( edgeHasLand( false, 0 ) ) walls.push_back( { { -0.5f * ( mapW + s ), 0.0f }, { s, mapD } } );
        if ( edgeHasLand( false, w - 1 ) ) walls.push_back( { { 0.5f * ( mapW + s ), 0.0f }, { s, mapD } } );
        return walls;
    }

}  // namespace NocInDan

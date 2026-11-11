#include "GroundTileDef.hh"

#include <array>
#include <string>

#include "Tomos/util/reflect/TReflectEnum.hh"

namespace NocInDan
{
    namespace
    {
        GroundTileDef makeDef( GroundTileId p_id, glm::vec3 p_tint, TileMovementRules p_movement = {} )
        {
            GroundTileDef def;
            def.m_id         = p_id;
            def.m_name       = std::string( Tomos::Reflect::enumNameLowerOr( p_id, "unknown" ) );
            def.m_albedoPath = "assets/tiles/" + def.m_name + ".png";
            def.m_tint       = p_tint;
            def.m_movement   = p_movement;
            return def;
        }

        constexpr TileMovementRules g_kImpassable{ .m_passable = false, .m_speedMultiplier = 0.0f };

        const std::vector<GroundTileDef> g_tiles = {
                makeDef( GroundTileId::Ocean, { 0.12f, 0.28f, 0.48f }, g_kImpassable ),
                makeDef( GroundTileId::Grass, { 0.40f, 0.62f, 0.32f } ),
                makeDef( GroundTileId::Forest, { 0.22f, 0.48f, 0.28f } ),
                makeDef( GroundTileId::Rocky, { 0.55f, 0.53f, 0.50f } ),
                makeDef( GroundTileId::Marsh, { 0.28f, 0.36f, 0.26f }, { .m_speedMultiplier = 0.85f } ),
                makeDef( GroundTileId::Sand, { 0.82f, 0.72f, 0.42f } ),
                makeDef( GroundTileId::Snow, { 0.80f, 0.86f, 0.92f } ),
                makeDef( GroundTileId::Road, { 0.42f, 0.36f, 0.30f }, { .m_speedMultiplier = 1.3f } ),
        };

        std::array<const GroundTileDef*, static_cast<size_t>( GroundTileId::Count )> buildLookup()
        {
            std::array<const GroundTileDef*, static_cast<size_t>( GroundTileId::Count )> table{};
            for ( const GroundTileDef& def : g_tiles )
            {
                const auto i = static_cast<size_t>( def.m_id );
                if ( i < table.size() ) table[ i ] = &def;
            }
            return table;
        }

        const auto g_byId = buildLookup();
    }  // namespace

    const std::vector<GroundTileDef>& builtinGroundTiles() { return g_tiles; }

    const GroundTileDef* findGroundTile( GroundTileId p_id )
    {
        const auto i = static_cast<size_t>( p_id );
        return i < g_byId.size() ? g_byId[ i ] : nullptr;
    }

    const TileMovementRules& movementRules( GroundTileId p_id )
    {
        const GroundTileDef* def = findGroundTile( p_id );
        return def != nullptr ? def->m_movement : g_kImpassable;
    }

}  // namespace NocInDan

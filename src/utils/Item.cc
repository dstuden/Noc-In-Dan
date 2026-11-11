#include "Item.hh"

namespace NocInDan
{

    void ItemRegistry::registerItem( Item p_item )
    {
        const size_t id = p_item.m_id;
        m_items.insert_or_assign( id, std::move( p_item ) );
    }

    bool ItemRegistry::has( size_t p_id ) const { return m_items.contains( p_id ); }

    const Item* ItemRegistry::get( size_t p_id ) const
    {
        const auto it = m_items.find( p_id );
        if ( it == m_items.end() ) return nullptr;
        return &it->second;
    }

    ItemRegistry& defaultItems()
    {
        static ItemRegistry reg = []
        {
            ItemRegistry r;
            r.registerItem( { .m_id = 1, .m_name = "Twigs", .m_description = "", .m_asset = "" } );
            r.registerItem( { .m_id = 2, .m_name = "Flint", .m_description = "", .m_asset = "" } );
            r.registerItem( { .m_id = 3, .m_name = "Berries", .m_description = "", .m_asset = "" } );
            return r;
        }();
        return reg;
    }

}  // namespace NocInDan

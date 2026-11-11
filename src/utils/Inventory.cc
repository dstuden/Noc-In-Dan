#include "Inventory.hh"

namespace NocInDan
{

    Inventory::Inventory( int p_capacity ) : m_slots( static_cast<size_t>( p_capacity > 0 ? p_capacity : 0 ) ) {}

    const InventorySlot& Inventory::slot( int p_index ) const { return m_slots[ static_cast<size_t>( p_index ) ]; }

    InventorySlot& Inventory::slot( int p_index ) { return m_slots[ static_cast<size_t>( p_index ) ]; }

    void Inventory::ensureCapacity( int p_capacity )
    {
        if ( p_capacity > size() ) m_slots.resize( static_cast<size_t>( p_capacity ) );
    }

    bool Inventory::setSlot( int p_index, size_t p_itemId, int p_amount )
    {
        if ( p_index < 0 || p_index >= size() ) return false;

        InventorySlot& s = m_slots[ static_cast<size_t>( p_index ) ];
        if ( p_amount <= 0 )
        {
            s = InventorySlot{};
            return true;
        }

        s.m_itemId = p_itemId;
        s.m_amount = p_amount;
        return true;
    }

    void Inventory::clearSlot( int p_index )
    {
        if ( p_index < 0 || p_index >= size() ) return;
        m_slots[ static_cast<size_t>( p_index ) ] = InventorySlot{};
    }

    int Inventory::findFirstEmpty() const
    {
        for ( int i = 0; i < size(); ++i )
        {
            if ( m_slots[ static_cast<size_t>( i ) ].isEmpty() ) return i;
        }
        return -1;
    }

    int Inventory::findFirstOf( size_t p_itemId ) const
    {
        for ( int i = 0; i < size(); ++i )
        {
            const InventorySlot& s = m_slots[ static_cast<size_t>( i ) ];
            if ( !s.isEmpty() && s.m_itemId == p_itemId ) return i;
        }
        return -1;
    }

    int Inventory::totalAmount( size_t p_itemId ) const
    {
        int total = 0;
        for ( const InventorySlot& s : m_slots )
        {
            if ( !s.isEmpty() && s.m_itemId == p_itemId ) total += s.m_amount;
        }
        return total;
    }

    int Inventory::add( size_t p_itemId, int p_amount )
    {
        if ( p_amount <= 0 ) return 0;

        for ( InventorySlot& s : m_slots )
        {
            if ( !s.isEmpty() && s.m_itemId == p_itemId )
            {
                s.m_amount += p_amount;
                return 0;
            }
        }

        const int empty = findFirstEmpty();
        if ( empty < 0 ) return p_amount;

        m_slots[ static_cast<size_t>( empty ) ] = InventorySlot{ p_itemId, p_amount };
        return 0;
    }

    int Inventory::remove( size_t p_itemId, int p_amount )
    {
        if ( p_amount <= 0 ) return 0;

        int removed = 0;
        for ( InventorySlot& s : m_slots )
        {
            if ( s.isEmpty() || s.m_itemId != p_itemId ) continue;

            const int take = s.m_amount < ( p_amount - removed ) ? s.m_amount : ( p_amount - removed );
            s.m_amount -= take;
            removed += take;

            if ( s.m_amount <= 0 ) s = InventorySlot{};
            if ( removed >= p_amount ) break;
        }
        return removed;
    }

}  // namespace NocInDan

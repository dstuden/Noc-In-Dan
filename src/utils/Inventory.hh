#pragma once

#include <cstddef>
#include <vector>

#include "Tomos/util/reflect/TReflectAttr.hh"

namespace NocInDan
{

    struct InventorySlot
    {
        size_t m_itemId{};
        int    m_amount{};

        bool isEmpty() const { return m_amount <= 0; }
    };

    class TOMOS_ANN( Tomos::Reflect::DataMeta{ "inventory", "Inventory" } ) Inventory
    {
    public:
        Inventory() = default;
        explicit Inventory( int p_capacity );

        int size() const { return static_cast<int>( m_slots.size() ); }

        const InventorySlot& slot( int p_index ) const;
        InventorySlot&       slot( int p_index );

        bool setSlot( int p_index, size_t p_itemId, int p_amount );
        void clearSlot( int p_index );

        int findFirstEmpty() const;
        int findFirstOf( size_t p_itemId ) const;

        int totalAmount( size_t p_itemId ) const;

        int add( size_t p_itemId, int p_amount );
        int remove( size_t p_itemId, int p_amount );

        void ensureCapacity( int p_capacity );

        std::vector<InventorySlot> m_slots;
    };

}  // namespace NocInDan

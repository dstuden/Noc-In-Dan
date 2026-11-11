#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>

namespace NocInDan
{

    struct Item
    {
        size_t      m_id{};
        std::string m_name;
        std::string m_description;
        std::string m_asset;
    };

    class ItemRegistry
    {
    public:
        void registerItem( Item p_item );

        bool has( size_t p_id ) const;

        const Item* get( size_t p_id ) const;

    private:
        std::unordered_map<size_t, Item> m_items;
    };

    ItemRegistry& defaultItems();

}  // namespace NocInDan

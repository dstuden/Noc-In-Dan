#pragma once

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace NocInDan
{

    // A source (item, debuff, buff) may own several keyed entries; result is the product of all.
    class SpeedModifierList
    {
    public:
        void set( std::string_view p_source, std::string_view p_key, float p_multiplier )
        {
            for ( Entry& e : m_entries )
            {
                if ( e.m_source == p_source && e.m_key == p_key )
                {
                    e.m_multiplier = p_multiplier;
                    return;
                }
            }
            m_entries.push_back( { std::string( p_source ), std::string( p_key ), p_multiplier } );
        }

        void remove( std::string_view p_source, std::string_view p_key )
        {
            std::erase_if( m_entries, [ & ]( const Entry& e ) { return e.m_source == p_source && e.m_key == p_key; } );
        }

        void removeSource( std::string_view p_source )
        {
            std::erase_if( m_entries, [ & ]( const Entry& e ) { return e.m_source == p_source; } );
        }

        void clear() { m_entries.clear(); }

        [[nodiscard]] float get() const
        {
            float m = 1.0f;
            for ( const Entry& e : m_entries ) m *= e.m_multiplier;
            return std::max( 0.0f, m );
        }

        [[nodiscard]] bool empty() const { return m_entries.empty(); }

    private:
        struct Entry
        {
            std::string m_source;
            std::string m_key;
            float       m_multiplier = 1.0f;
        };

        std::vector<Entry> m_entries;
    };

}  // namespace NocInDan

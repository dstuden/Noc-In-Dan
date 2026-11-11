#pragma once

#include <string>
#include <vector>

#include "PrefabDef.hh"

namespace NocInDan
{

    class PrefabCatalog
    {
    public:
        static PrefabCatalog& get();

        // Load every *.json in p_directory (call once at startup).
        void load( const std::string& p_directory = "assets/prefabs" );

        [[nodiscard]] const PrefabDef* find( const std::string& p_id ) const;

    private:
        PrefabCatalog() = default;

        std::vector<PrefabDef> m_defs;
    };

}  // namespace NocInDan

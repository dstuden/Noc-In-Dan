#include "PrefabCatalog.hh"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include "Tomos/util/logger/TLogger.hh"
#include "Tomos/util/path/TPath.hh"
#include "Tomos/util/reflect/TReflectFields.hh"

namespace NocInDan
{
    namespace
    {
        // The filename stem is the prefab id.
        bool loadPrefabFile( const std::filesystem::path& p_path, PrefabDef& p_out )
        {
            std::ifstream  file( p_path );
            nlohmann::json j = nlohmann::json::parse( file, nullptr, /*allow_exceptions=*/false );
            if ( !j.is_object() )
            {
                TLOG_ERROR() << "[PrefabCatalog] '" << p_path.string() << "' is not a JSON object";
                return false;
            }

            Tomos::Reflect::loadFields( p_out, j );
            p_out.m_id = p_path.stem().string();
            return true;
        }
    }  // namespace

    PrefabCatalog& PrefabCatalog::get()
    {
        static PrefabCatalog s_catalog;
        return s_catalog;
    }

    void PrefabCatalog::load( const std::string& p_directory )
    {
        m_defs.clear();

        const std::filesystem::path dir = Tomos::TPath::resolve( p_directory );
        std::error_code             ec;
        for ( const auto& entry : std::filesystem::directory_iterator( dir, ec ) )
        {
            if ( !entry.is_regular_file() || entry.path().extension() != ".json" ) continue;
            if ( PrefabDef def; loadPrefabFile( entry.path(), def ) ) m_defs.push_back( std::move( def ) );
        }

        if ( ec ) TLOG_ERROR() << "[PrefabCatalog] cannot read '" << dir.string() << "': " << ec.message();
        TLOG_INFO() << "[PrefabCatalog] loaded " << m_defs.size() << " prefab(s) from '" << dir.string() << "'";
    }

    const PrefabDef* PrefabCatalog::find( const std::string& p_id ) const
    {
        for ( const PrefabDef& def : m_defs )
            if ( def.m_id == p_id ) return &def;
        return nullptr;
    }

}  // namespace NocInDan

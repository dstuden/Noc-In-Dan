#include "App.hh"

#include <memory>

#include "RegisterGameTypes.hh"
#include "layers/GameLayer.hh"
#include "layers/UiLayer.hh"
#include "prefabs/PrefabCatalog.hh"

namespace NocInDan
{
    App::App() : TApplication( Tomos::TWindowProps{ "Noč In Dan", 1280, 720 } )
    {
        initGpu( TOMOS_DEBUG != 0 );
        registerGameTypes();
        PrefabCatalog::get().load();
        pushLayer( std::make_unique<GameLayer>() );
        pushOverlay( std::make_unique<UiLayer>() );
    }
}  // namespace NocInDan

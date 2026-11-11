#include "RegisterGameTypes.hh"

#include "Tomos/systems/TComponentRegistry.hh"
#include "Tomos/systems/script/TScriptRegistry.hh"
#include "Tomos/util/reflect/TReflectComponent.hh"
#include "components/TiltedCameraComponent.hh"
#include "scripts/MobScript.hh"
#include "scripts/PlayerScript.hh"
#include "scripts/SunOrbitScript.hh"
#include "scripts/WorldScript.hh"

namespace NocInDan
{
    void registerGameTypes()
    {
        auto& components = Tomos::TComponentRegistry::get();

        components.registerType( Tomos::Reflect::makePodComponent<TiltedCameraComponent>() );

        auto& scripts = Tomos::TScriptRegistry::get();
        scripts.registerType<PlayerScript>( "Player" );
        scripts.registerType<MobScript>( "Mob" );
        scripts.registerType<WorldScript>( "World" );
        scripts.registerType<SunOrbitScript>( "SunOrbit" );
    }
}  // namespace NocInDan

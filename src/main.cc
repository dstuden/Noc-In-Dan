#include <iostream>
#include <string>

#include "App.hh"
#include "Tomos/util/cli/TCliArgs.hh"
#include "Tomos/util/path/TPath.hh"

int main( int argc, char** argv )
{
    std::string configPath;

    Tomos::TCliArgs args;
    args.addOption( "--config", &configPath, "Path to tomos.json" );

    if ( !args.parse( argc, argv ) || args.helpRequested() )
    {
        std::cout << args.helpText( argv[ 0 ] != nullptr ? argv[ 0 ] : "NocInDan" );
        return args.helpRequested() ? 0 : 1;
    }

    if ( !configPath.empty() ) Tomos::TPath::setConfigPath( configPath );

    NocInDan::App app;
    app.run();
    return 0;
}

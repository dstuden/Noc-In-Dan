#include "GameLayer.hh"

#include "../intro/Intro.hh"
#include "Tomos/core/app/TApplication.hh"
#include "Tomos/util/logger/TLogger.hh"

namespace NocInDan
{
    GameLayer::GameLayer() : TSceneLayer( "Game" ) {}

    void GameLayer::onAttach()
    {
        TSceneLayer::onAttach();

        auto* gpu = Tomos::TApplication::get().gpu();
        if ( gpu == nullptr )
        {
            TLOG_ERROR() << "[GameLayer] no GPU";
            return;
        }
        spawnIntro( scene(), *gpu );
    }
}  // namespace NocInDan

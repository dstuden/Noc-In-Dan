#pragma once

#include "Tomos/core/layers/TSceneLayer.hh"

namespace NocInDan
{

    class GameLayer : public Tomos::TSceneLayer
    {
    public:
        GameLayer();

        void onAttach() override;
    };

}  // namespace NocInDan

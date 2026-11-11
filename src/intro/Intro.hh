#pragma once

#include "Tomos/core/scene/TScene.hh"

namespace Tomos
{
    class TVkGpu;
}

namespace NocInDan
{

    // Banner sprite with fade in/out. Space/Esc hard-cuts to InGameScene.
    void spawnIntro( Tomos::TScene& p_scene, Tomos::TVkGpu& p_gpu );

}  // namespace NocInDan

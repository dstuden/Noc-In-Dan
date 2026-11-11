#pragma once

#include "WorldMap.hh"

namespace Tomos
{
    class TScene;
    class TSceneNode;
    class TVkGpu;
}  // namespace Tomos

namespace NocInDan
{
    // Ground slab collider, ocean + land chunk meshes and coast walls for p_map, as children of
    // p_parent. Derived from the seed, so the caller should keep p_parent transient.
    void buildWorldGeometry( Tomos::TScene& p_scene, Tomos::TVkGpu& p_gpu, Tomos::TSceneNode& p_parent, const WorldMap& p_map );

}  // namespace NocInDan

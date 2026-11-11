#pragma once

#include <cstdint>
#include <span>

#include "../worldgen/WorldMap.hh"

namespace Tomos
{
    class TScene;
    class TSceneNode;
    class TVkGpu;
}  // namespace Tomos

namespace NocInDan
{

    // Spawns p_ids (indices into p_map.m_spawns) under p_parent: a node with sprite + collider for
    // each prefab that has a collider, one instanced-sprite node per decorative prefab type.
    void buildSpawnChunk( Tomos::TScene& p_scene, Tomos::TVkGpu& p_gpu, Tomos::TSceneNode& p_parent, const WorldMap& p_map, std::span<const uint32_t> p_ids );

}  // namespace NocInDan

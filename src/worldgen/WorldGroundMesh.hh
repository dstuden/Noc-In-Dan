#pragma once

#include <cstddef>
#include <memory>
#include <span>
#include <vector>

#include "Tomos/gpu/vulkan/TVkMesh.hh"
#include "WorldMap.hh"

namespace Tomos
{
    class TVkGpu;
}

namespace NocInDan
{

    // Ground layer table entry, indexed by GroundTileId (matches GroundLayer in ground.frag).
    struct GroundLayerGpu
    {
        glm::vec4 m_tint{ 1.0f };
        uint32_t  m_texSlot   = 0;  // bindless texture slot; 0 = white default
        float     m_uvScale   = 1.0f;
        float     m_roughness = 0.85f;
        uint32_t  m_blend     = 0;  // 0 = never blended (ocean, void)
    };
    static_assert( sizeof( GroundLayerGpu ) == 32 );

    // Ground storage buffer: header, layer table, then u8 ground ids packed four per uint.
    [[nodiscard]] std::vector<std::byte> buildGroundShaderData( const WorldMap& p_world, std::span<const GroundLayerGpu> p_layers );

    // Land cells of one chunk (UVs = cell coordinates); nullptr if the chunk has no land.
    [[nodiscard]] std::unique_ptr<Tomos::TVkMesh> makeLandChunkMesh( Tomos::TVkGpu& p_gpu, const WorldMap& p_world, int p_chunkX, int p_chunkY );

    // Merged quads over every non-land cell (land chunks cover the rest); nullptr if all land.
    [[nodiscard]] std::unique_ptr<Tomos::TVkMesh> makeOceanMesh( Tomos::TVkGpu& p_gpu, const WorldMap& p_world );

}  // namespace NocInDan

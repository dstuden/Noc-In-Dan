#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "../worldgen/WorldMap.hh"
#include "Tomos/systems/script/TScript.hh"
#include "Tomos/util/memory/THandle.hh"

namespace NocInDan
{
    class InGameScene;

    // Owns the generated world. A save holds only the seed and the spawns removed since generation;
    // onAttach (new game or load) regenerates the map, ground, walls and spawns from them, all on
    // transient nodes.
    class WorldScript : public Tomos::TScript
    {
    public:
        WorldScript() = default;
        explicit WorldScript( uint64_t p_seed ) : m_seed( p_seed ) {}

        void onAttach() override;
        void onDetach() override;

        [[nodiscard]] const WorldMap* map() const { return m_map.get(); }

        // Take spawn p_id (index into map()->m_spawns) out of the world for good, e.g. a felled
        // tree or picked grass. Rebuilds only that spawn's chunk.
        void removeSpawn( uint32_t p_id );

        uint64_t              m_seed = 0;
        std::vector<uint32_t> m_removedSpawns;  // sorted

    private:
        void buildChunk( size_t p_chunk );

        InGameScene*                       m_scene = nullptr;
        std::unique_ptr<WorldMap>          m_map;
        std::vector<std::vector<uint32_t>> m_chunkSpawns;  // spawn ids per chunk
        std::vector<Tomos::TNodeHandle>    m_chunkNodes;
        Tomos::TNodeHandle                 m_spawnRoot{};
    };
}  // namespace NocInDan

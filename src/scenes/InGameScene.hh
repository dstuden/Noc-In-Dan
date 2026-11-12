#pragma once

#include "Tomos/core/scene/TScene.hh"

namespace NocInDan
{
    class PlayerScript;
    class SunOrbitScript;
    class WorldScript;
    struct Survival;
    struct WorldClock;
    struct WorldMap;

    class InGameScene : public Tomos::TScene
    {
    public:
        InGameScene();

        void onSystemsReady() override;

        // Scene p_node is attached to, or null outside an InGameScene.
        [[nodiscard]] static InGameScene* of( Tomos::TSceneNode& p_node );

        [[nodiscard]] const WorldMap*   worldMap() const;
        [[nodiscard]] const Survival*   survival() const;
        [[nodiscard]] const WorldClock* clock() const;

        // Set by the scripts themselves on attach/detach, so they stay right across a load.
        PlayerScript*   m_player = nullptr;
        SunOrbitScript* m_sun    = nullptr;
        WorldScript*    m_world  = nullptr;

    private:
        void createPlayer();
        void createWorld();
        void createLight();
        void createCampfire();
        void spawnMobs();
    };
}  // namespace NocInDan

#pragma once

#include "../utils/GameCalendar.hh"
#include "../utils/WorldClock.hh"
#include "Tomos/systems/script/TScript.hh"

namespace Tomos
{
    class TLightComponent;
    class TColorGradeComponent;
}  // namespace Tomos

namespace NocInDan
{
    class InGameScene;

    // Drives the sun (this node's directional light), the child Ambient light and the color grade
    // from the saved WorldClock. Add it after those components.
    class SunOrbitScript : public Tomos::TScript
    {
    public:
        void onAttach() override;
        void onDetach() override;
        void update( float p_dt ) override;

        WorldClock m_clock{};

    private:
        InGameScene*                 m_scene   = nullptr;
        Tomos::TLightComponent*      m_light   = nullptr;
        Tomos::TLightComponent*      m_ambient = nullptr;
        Tomos::TColorGradeComponent* m_grade   = nullptr;
    };
}  // namespace NocInDan

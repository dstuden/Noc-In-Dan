#pragma once

#include "GameCalendar.hh"
#include "Tomos/util/reflect/TReflectAttr.hh"

namespace NocInDan
{
    struct TOMOS_ANN( Tomos::Reflect::DataMeta{ "worldClock", "World Clock" } ) WorldClock
    {
        float  m_timeOfDay   = 0.25f;
        int    m_dayCount    = 1;
        int    m_year        = 1;
        Season m_season      = Season::Autumn;
        int    m_dayInSeason = 0;

        // derived each tick; not saved
        TOMOS_ANN( Tomos::Reflect::Skip{} ) DayPhase m_phase = DayPhase::Day;
    };
}  // namespace NocInDan

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <string_view>

#include "Tomos/gpu/TColorGrade.hh"
#include "Tomos/util/reflect/TReflectEnum.hh"

namespace NocInDan
{
    // World clock: fixed-length days of 16 segments; seasons redistribute day / dusk / night
    // among those segments and tint the sun.

    enum class Season : uint8_t
    {
        Autumn = 0,
        Winter = 1,
        Spring = 2,
        Summer = 3,
    };

    enum class DayPhase : uint8_t
    {
        Day   = 0,
        Dusk  = 1,
        Night = 2,
    };

    struct ClockSegs
    {
        int m_day   = 10;
        int m_dusk  = 4;
        int m_night = 2;
    };

    // Everything the time of day drives: sun light, ambient fill and color grade.
    struct SunLook
    {
        glm::vec3          m_direction{ 0.0f, -1.0f, 0.0f };
        glm::vec3          m_color{ 1.0f };
        float              m_intensity  = 1.0f;
        bool               m_castShadow = true;
        DayPhase           m_phase      = DayPhase::Day;
        glm::vec3          m_ambient{ 0.0f };
        Tomos::TColorGrade m_grade{};
    };

    struct GameCalendar
    {
        static constexpr int   g_kSegsPerDay  = 16;
        static constexpr float g_kSegSeconds  = 4.0f;
        static constexpr float g_kDaySeconds  = static_cast<float>( g_kSegsPerDay ) * g_kSegSeconds;
        static constexpr float g_kSunDistance = 140.0f;
        static constexpr int   g_kSeasonCount = 4;

        // Days per season.
        static constexpr int g_kSeasonLengths[ g_kSeasonCount ] = { 20, 15, 20, 15 };

        static constexpr ClockSegs g_kSeasonSegs[ g_kSeasonCount ] = {
                ClockSegs{ 10, 4, 2 },  // Autumn
                ClockSegs{ 6, 4, 6 },  // Winter - short day
                ClockSegs{ 10, 4, 2 },  // Spring
                ClockSegs{ 12, 3, 1 },  // Summer - long day
        };

        [[nodiscard]] static constexpr int seasonLength( Season p_season ) { return g_kSeasonLengths[ static_cast<int>( p_season ) ]; }

        [[nodiscard]] static constexpr ClockSegs segsFor( Season p_season ) { return g_kSeasonSegs[ static_cast<int>( p_season ) ]; }

        [[nodiscard]] static constexpr std::string_view seasonName( Season p_season ) { return Tomos::Reflect::enumNameOr( p_season, "Autumn" ); }

        [[nodiscard]] static constexpr std::string_view phaseName( DayPhase p_phase ) { return Tomos::Reflect::enumNameOr( p_phase, "Day" ); }

        [[nodiscard]] static Season nextSeason( Season p_season ) { return static_cast<Season>( ( static_cast<int>( p_season ) + 1 ) % g_kSeasonCount ); }

        struct SeasonSun
        {
            float     m_maxElevation;  // radians
            float     m_noonDeclination;  // radians
            glm::vec3 m_dayColor;
            glm::vec3 m_duskColor;
            float     m_dayIntensity;
            float     m_duskIntensity;
        };

        static constexpr SeasonSun g_kSeasonSun[ g_kSeasonCount ] = {
                { glm::radians( 58.0f ), glm::radians( -14.0f ), { 1.00f, 0.92f, 0.72f }, { 1.00f, 0.42f, 0.16f }, 1.55f, 0.95f },  // Autumn
                { glm::radians( 42.0f ), glm::radians( -32.0f ), { 0.82f, 0.90f, 1.00f }, { 0.85f, 0.50f, 0.35f }, 1.25f, 0.70f },  // Winter
                { glm::radians( 62.0f ), glm::radians( 16.0f ), { 0.95f, 0.98f, 0.88f }, { 1.00f, 0.55f, 0.35f }, 1.50f, 0.90f },  // Spring
                { glm::radians( 78.0f ), glm::radians( 38.0f ), { 1.00f, 0.96f, 0.78f }, { 1.00f, 0.35f, 0.12f }, 1.85f, 1.05f },  // Summer
        };

        [[nodiscard]] static DayPhase phaseAt( float p_timeOfDay, ClockSegs p_segs, float* p_phaseT = nullptr )
        {
            const float tod     = std::clamp( p_timeOfDay, 0.0f, 0.999999f );
            const float dayEnd  = static_cast<float>( p_segs.m_day ) / static_cast<float>( g_kSegsPerDay );
            const float duskEnd = static_cast<float>( p_segs.m_day + p_segs.m_dusk ) / static_cast<float>( g_kSegsPerDay );

            if ( tod < dayEnd )
            {
                if ( p_phaseT ) *p_phaseT = ( dayEnd > 1e-6f ) ? ( tod / dayEnd ) : 0.0f;
                return DayPhase::Day;
            }
            if ( tod < duskEnd )
            {
                if ( p_phaseT ) *p_phaseT = ( duskEnd > dayEnd ) ? ( ( tod - dayEnd ) / ( duskEnd - dayEnd ) ) : 0.0f;
                return DayPhase::Dusk;
            }
            if ( p_phaseT ) *p_phaseT = ( duskEnd < 1.0f ) ? ( ( tod - duskEnd ) / ( 1.0f - duskEnd ) ) : 0.0f;
            return DayPhase::Night;
        }

        [[nodiscard]] static int advanceTimeOfDay( float& p_timeOfDay, float p_dt )
        {
            p_timeOfDay += p_dt / g_kDaySeconds;
            int crossed = 0;
            while ( p_timeOfDay >= 1.0f )
            {
                p_timeOfDay -= 1.0f;
                ++crossed;
            }
            return crossed;
        }

        static void advanceDays( Season& p_season, int& p_dayInSeason, int& p_dayCount, int& p_year, int p_days )
        {
            for ( int i = 0; i < p_days; ++i )
            {
                ++p_dayCount;
                ++p_dayInSeason;
                if ( p_dayInSeason >= seasonLength( p_season ) )
                {
                    p_dayInSeason = 0;
                    p_season      = nextSeason( p_season );
                    if ( p_season == Season::Autumn ) ++p_year;
                }
            }
        }

        [[nodiscard]] static SunLook evaluateSun( Season p_season, float p_timeOfDay )
        {
            const ClockSegs segs   = segsFor( p_season );
            float           phaseT = 0.0f;
            const DayPhase  phase  = phaseAt( p_timeOfDay, segs, &phaseT );

            const SeasonSun& sun = g_kSeasonSun[ static_cast<int>( p_season ) ];

            const float litEnd = static_cast<float>( segs.m_day + segs.m_dusk ) / static_cast<float>( g_kSegsPerDay );
            const float litT   = ( litEnd > 1e-6f ) ? std::clamp( p_timeOfDay / litEnd, 0.0f, 1.0f ) : 1.0f;
            const float theta  = litT * glm::pi<float>();
            const float rise   = std::sin( theta );

            const float noonY = std::tan( sun.m_maxElevation );
            const float noonZ = std::tan( sun.m_noonDeclination );

            const glm::vec3 sunArc( -std::cos( theta ), rise * noonY, rise * noonZ );

            const float     dip = std::sin( phaseT * glm::pi<float>() );
            const glm::vec3 nightArc( 1.0f - 2.0f * phaseT, -0.20f * dip, -0.25f * noonZ * dip );

            const float     horizonI    = sun.m_duskIntensity * 0.30f;
            constexpr float kNightFloor = 0.02f;

            const glm::vec3 horizonColor = glm::mix( sun.m_duskColor, sun.m_dayColor, 0.25f );
            const glm::vec3 nightColor   = sun.m_duskColor * 0.55f;

            SunLook look;
            look.m_phase      = phase;
            look.m_castShadow = true;  // always on -> no shadow pop at dawn/dusk

            if ( phase == DayPhase::Night )
            {
                look.m_direction = glm::normalize( -nightArc );
                look.m_color     = glm::mix( horizonColor, nightColor, dip );
                look.m_intensity = glm::mix( horizonI, kNightFloor, dip );
            }
            else
            {
                look.m_direction = glm::normalize( -sunArc );
                look.m_color     = glm::mix( horizonColor, sun.m_dayColor, rise );
                look.m_intensity = glm::mix( horizonI, sun.m_dayIntensity, rise );
            }

            float darkness = 0.0f;
            if ( phase == DayPhase::Dusk )
            {
                darkness = phaseT;
            }
            else if ( phase == DayPhase::Night )
            {
                darkness = 1.0f;
            }
            else  // DayPhase::Day
            {
                constexpr float kDawnFadeSeconds = 20.0f;
                const float dawnFrac = std::clamp( kDawnFadeSeconds / ( g_kDaySeconds * ( static_cast<float>( segs.m_day ) / g_kSegsPerDay ) ), 0.01f, 0.5f );
                darkness             = 1.0f - glm::smoothstep( 0.0f, dawnFrac, phaseT );
            }

            applyMood( look, p_season, darkness );
            return look;
        }

    private:
        struct Mood
        {
            glm::vec3          m_ambient;
            Tomos::TColorGrade m_grade;
        };

        // Per-phase presets: warm day, amber/purple dusk, cold dark night.
        static constexpr Mood g_kDayMood{ { 0.09f, 0.085f, 0.075f },
                                          { 1.0f, 1.05f, 0.9f, { 0.01f, 0.01f, 0.01f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.98f, 0.95f }, 0.2f } };
        static constexpr Mood g_kDuskMood{ { 0.06f, 0.04f, 0.035f },
                                           { 1.1f, 1.12f, 0.85f, { 0.03f, 0.0f, 0.04f }, { 1.0f, 0.97f, 0.95f }, { 1.08f, 0.92f, 0.8f }, 0.4f } };
        static constexpr Mood g_kNightMood{ { 0.012f, 0.016f, 0.035f },
                                            { 1.3f, 1.15f, 0.55f, { 0.0f, 0.01f, 0.04f }, { 0.95f, 0.97f, 1.05f }, { 0.75f, 0.85f, 1.0f }, 0.65f } };

        // Highlight tint and saturation scale per season.
        static constexpr glm::vec3 g_kSeasonTint[ g_kSeasonCount ] = {
                { 1.02f, 0.99f, 0.94f },  // Autumn
                { 0.92f, 0.97f, 1.06f },  // Winter
                { 0.97f, 1.03f, 0.95f },  // Spring
                { 1.05f, 1.00f, 0.92f },  // Summer
        };
        static constexpr float g_kSeasonSaturation[ g_kSeasonCount ] = { 1.0f, 0.85f, 1.0f, 1.05f };

        // Blend Day -> Dusk -> Night on one continuous darkness value.
        static void applyMood( SunLook& p_look, Season p_season, float p_darkness )
        {
            const float d = std::clamp( p_darkness, 0.0f, 1.0f );
            const float t = glm::smoothstep( 0.0f, 1.0f, d );

            const Mood* from;
            const Mood* to;
            float       w;
            if ( t < 0.5f )
            {
                from = &g_kDayMood;
                to   = &g_kDuskMood;
                w    = t * 2.0f;
            }
            else
            {
                from = &g_kDuskMood;
                to   = &g_kNightMood;
                w    = ( t - 0.5f ) * 2.0f;
            }

            const auto      s    = static_cast<int>( p_season );
            const glm::vec3 tint = g_kSeasonTint[ s ];
            p_look.m_ambient     = glm::mix( from->m_ambient, to->m_ambient, w ) * tint;
            p_look.m_grade       = Tomos::TColorGrade::mix( from->m_grade, to->m_grade, w );
            p_look.m_grade.m_gain *= tint;
            p_look.m_grade.m_saturation *= g_kSeasonSaturation[ s ];
        }
    };
}  // namespace NocInDan

#include "SunOrbitScript.hh"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../scenes/InGameScene.hh"
#include "Tomos/core/scene/TSceneNode.hh"
#include "Tomos/systems/light/TLightComponent.hh"
#include "Tomos/systems/post/TColorGradeComponent.hh"

namespace NocInDan
{
    void SunOrbitScript::onAttach()
    {
        m_light = node().findComponent<Tomos::TLightComponent>();
        m_grade = node().findComponent<Tomos::TColorGradeComponent>();
        for ( Tomos::TSceneNode* child : node().getChildren() )
        {
            auto* light = child->findComponent<Tomos::TLightComponent>();
            if ( light != nullptr && light->m_type == Tomos::TLightType::Ambient ) m_ambient = light;
        }

        m_scene = InGameScene::of( node() );
        if ( m_scene != nullptr ) m_scene->m_sun = this;

        update( 0.0f );  // apply the saved time of day before the first frame
    }

    void SunOrbitScript::onDetach()
    {
        if ( m_scene != nullptr && m_scene->m_sun == this ) m_scene->m_sun = nullptr;
    }

    void SunOrbitScript::update( float p_dt )
    {
        const int crossed = GameCalendar::advanceTimeOfDay( m_clock.m_timeOfDay, p_dt );
        if ( crossed > 0 ) GameCalendar::advanceDays( m_clock.m_season, m_clock.m_dayInSeason, m_clock.m_dayCount, m_clock.m_year, crossed );

        const SunLook look = GameCalendar::evaluateSun( m_clock.m_season, m_clock.m_timeOfDay );
        m_clock.m_phase    = look.m_phase;

        const glm::quat rot = glm::quatLookAt( look.m_direction, glm::vec3( 0.0f, 0.0f, 1.0f ) );
        node().m_transform.setLocalTRS( -look.m_direction * GameCalendar::g_kSunDistance, rot, glm::vec3( 1.0f ) );

        if ( m_light != nullptr )
        {
            m_light->m_color      = look.m_color;
            m_light->m_intensity  = look.m_intensity;
            m_light->m_castShadow = look.m_castShadow;
        }
        if ( m_ambient != nullptr ) m_ambient->m_color = look.m_ambient;
        if ( m_grade != nullptr ) m_grade->setGrade( look.m_grade );
    }
}  // namespace NocInDan

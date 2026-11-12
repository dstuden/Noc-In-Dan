#include "CampfireScript.hh"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "Tomos/core/scene/TSceneNode.hh"
#include "Tomos/systems/light/TLightComponent.hh"
#include "Tomos/systems/sprite/TSpriteComponent.hh"

namespace NocInDan
{
    void CampfireScript::onAttach()
    {
        m_sprite = node().findComponent<Tomos::TSpriteComponent>();

        m_light = node().findComponent<Tomos::TLightComponent>();
        for ( Tomos::TSceneNode* child : node().getChildren() )
        {
            if ( m_light != nullptr ) break;
            m_light = child->findComponent<Tomos::TLightComponent>();
        }
        if ( m_light != nullptr ) m_baseIntensity = m_light->m_intensity;

        m_time = static_cast<float>( reinterpret_cast<uintptr_t>( &node() ) % 97u ) * 0.173f;
        applyFrame( 0 );
    }

    void CampfireScript::update( float p_dt )
    {
        m_time += p_dt;

        if ( m_frameCount > 0 && m_fps > 0.0f )
        {
            const int frame = static_cast<int>( m_time * m_fps ) % m_frameCount;
            if ( frame != m_frame ) applyFrame( frame );
        }

        if ( m_light != nullptr )
        {
            const float t        = m_time;
            const float noise    = 0.55f * std::sin( t * 7.3f ) + 0.30f * std::sin( t * 13.1f + 1.7f ) + 0.15f * std::sin( t * 23.7f + 0.4f );
            m_light->m_intensity = m_baseIntensity * std::max( 0.0f, 1.0f + m_flickerDepth * noise );
        }
    }

    void CampfireScript::applyFrame( int p_frame )
    {
        m_frame = p_frame;
        if ( m_sprite == nullptr || m_frameCount <= 0 ) return;

        const float w     = 1.0f / static_cast<float>( m_frameCount );
        m_sprite->m_uvMin = { w * static_cast<float>( p_frame ), 0.0f };
        m_sprite->m_uvMax = { w * static_cast<float>( p_frame + 1 ), 1.0f };
    }
}  // namespace NocInDan

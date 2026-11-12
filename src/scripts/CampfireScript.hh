#pragma once

#include "Tomos/systems/script/TScript.hh"

namespace Tomos
{
    class TLightComponent;
    class TSpriteComponent;
}  // namespace Tomos

namespace NocInDan
{
    class CampfireScript : public Tomos::TScript
    {
    public:
        void onAttach() override;
        void update( float p_dt ) override;

        int   m_frameCount = 6;
        float m_fps        = 9.0f;

        float m_baseIntensity = 6.0f;
        float m_flickerDepth  = 0.3f;

    private:
        void applyFrame( int p_frame );

        Tomos::TSpriteComponent* m_sprite = nullptr;
        Tomos::TLightComponent*  m_light  = nullptr;

        float m_time  = 0.0f;
        int   m_frame = -1;
    };
}  // namespace NocInDan

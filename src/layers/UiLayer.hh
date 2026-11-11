#pragma once

#include <glm/vec4.hpp>
#include <memory>

#include "Tomos/gpu/vulkan/TVkImage.hh"
#include "Tomos/ui/TUiLayer.hh"
#include "Tomos/ui/canvas/TCanvas.hh"
#include "Tomos/ui/canvas/TCanvasBackend.hh"
#include "Tomos/ui/canvas/TFont.hh"

namespace NocInDan
{

    struct Survival;
    struct WorldClock;

    class UiLayer : public Tomos::TUiLayer
    {
    public:
        UiLayer();

        void onAttach() override;
        void onDetach() override;
        void onUi( float p_dt ) override;

    private:
        [[nodiscard]] Tomos::TCanvasBackend& canvasBackend();
        void                                 loadAssets();
        void                                 releaseAssets();

        void drawMeter( Tomos::TCanvas& p_canvas, float p_x, float p_y, float p_w, float p_h, float p_pct, const glm::vec4& p_fill, const char* p_label );
        void drawInventory( Tomos::TCanvas& p_canvas, const Survival& p_survival );
        void drawClock( Tomos::TCanvas& p_canvas, const WorldClock& p_clock );

        std::unique_ptr<Tomos::TFont>    m_font;
        std::unique_ptr<Tomos::TVkImage> m_panel;
        std::unique_ptr<Tomos::TVkImage> m_slot;
        std::unique_ptr<Tomos::TVkImage> m_border;
    };

}  // namespace NocInDan

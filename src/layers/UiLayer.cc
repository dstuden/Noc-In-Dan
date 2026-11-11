#include "UiLayer.hh"

#include <algorithm>
#include <cstdio>

#include "../scenes/InGameScene.hh"
#include "../utils/GameCalendar.hh"
#include "../utils/Item.hh"
#include "../utils/Survival.hh"
#include "../utils/WorldClock.hh"
#include "Tomos/core/app/TApplication.hh"
#include "Tomos/core/layers/TSceneLayer.hh"
#include "Tomos/util/image/TImageLoad.hh"
#include "Tomos/util/logger/TLogger.hh"

namespace NocInDan
{
    namespace
    {
        constexpr float                 kSlice = 16.0f;
        constexpr Tomos::TCanvasMargins kMargins{ kSlice, kSlice, kSlice, kSlice };

        float statPct( const Stat<float>& p_s )
        {
            const float span = p_s.m_maxValue - p_s.m_minValue;
            if ( span <= 0.0f ) return 0.0f;
            return std::clamp( ( p_s.m_currentValue - p_s.m_minValue ) / span, 0.0f, 1.0f );
        }
    }  // namespace

    UiLayer::UiLayer() : TUiLayer( std::make_unique<Tomos::TCanvasBackend>(), "HUD" ) {}

    Tomos::TCanvasBackend& UiLayer::canvasBackend() { return static_cast<Tomos::TCanvasBackend&>( backend() ); }

    void UiLayer::onAttach()
    {
        TUiLayer::onAttach();
        loadAssets();
    }

    void UiLayer::onDetach()
    {
        TUiLayer::onDetach();
        releaseAssets();
    }

    void UiLayer::releaseAssets()
    {
        m_font.reset();
        Tomos::TVkGpu* gpu = Tomos::TApplication::get().gpu();
        for ( auto* img : { &m_panel, &m_slot, &m_border } )
        {
            if ( *img != nullptr && gpu != nullptr ) gpu->retire( std::move( **img ) );
            img->reset();
        }
    }

    void UiLayer::loadAssets()
    {
        Tomos::TVkGpu* gpu = Tomos::TApplication::get().gpu();
        if ( gpu == nullptr ) return;

        m_font = Tomos::TFont::load( *gpu, "assets/ui/fonts/kenpixel_mini.ttf", 32.0f );
        if ( m_font == nullptr ) TLOG_WARN() << "[UiLayer] font load failed";

        auto load = [ gpu ]( const char* p_path ) { return Tomos::createSampledImageFromFile( *gpu, p_path, Tomos::TTexFilter::Nearest ); };

        m_panel  = load( "assets/ui/debug/fantasy-ui-borders/PNG/Panel/panel-013.png" );
        m_slot   = load( "assets/ui/debug/fantasy-ui-borders/PNG/Transparent center/panel-transparent-center-013.png" );
        m_border = load( "assets/ui/debug/fantasy-ui-borders/PNG/Border/panel-border-013.png" );
    }

    void UiLayer::drawMeter( Tomos::TCanvas& p_canvas, float p_x, float p_y, float p_w, float p_h, float p_pct, const glm::vec4& p_fill, const char* p_label )
    {
        const float pad = 8.0f;
        p_canvas.rect( p_x + pad, p_y + pad, ( p_w - pad * 2.0f ) * p_pct, p_h - pad * 2.0f, p_fill );

        if ( m_border != nullptr ) p_canvas.nineSlice( m_border.get(), p_x, p_y, p_w, p_h, kMargins );

        if ( m_font != nullptr && p_label != nullptr ) p_canvas.text( *m_font, p_label, p_x + 14.0f, p_y + p_h * 0.5f - 8.0f, 16.0f, glm::vec4( 1.0f ) );
    }

    void UiLayer::drawInventory( Tomos::TCanvas& p_canvas, const Survival& p_survival )
    {
        constexpr int kCols = 5;
        constexpr int kRows = g_kInventorySlots / kCols;
        static_assert( kCols * kRows == g_kInventorySlots );
        constexpr float kSlot  = 56.0f;
        constexpr float kGap   = 4.0f;
        const float     panelW = kCols * kSlot + ( kCols + 1 ) * kGap + 16.0f;
        const float     panelH = kRows * kSlot + ( kRows + 1 ) * kGap + 16.0f;
        const float     panelX = ( p_canvas.width() - panelW ) * 0.5f;
        const float     panelY = p_canvas.height() - panelH - 16.0f;

        if ( m_panel != nullptr ) p_canvas.nineSlice( m_panel.get(), panelX, panelY, panelW, panelH, kMargins, glm::vec4( 1.0f, 1.0f, 1.0f, 0.92f ) );

        ItemRegistry& items = defaultItems();

        for ( int i = 0; i < g_kInventorySlots; ++i )
        {
            const int   col = i % kCols;
            const int   row = i / kCols;
            const float sx  = panelX + 12.0f + kGap + col * ( kSlot + kGap );
            const float sy  = panelY + 12.0f + kGap + row * ( kSlot + kGap );

            if ( m_slot != nullptr )
                p_canvas.nineSlice( m_slot.get(), sx, sy, kSlot, kSlot, kMargins );
            else
                p_canvas.rect( sx, sy, kSlot, kSlot, glm::vec4( 0.15f, 0.12f, 0.1f, 0.85f ) );

            if ( i >= p_survival.m_inventory.size() ) continue;
            const InventorySlot& invSlot = p_survival.m_inventory.slot( i );
            if ( invSlot.isEmpty() ) continue;

            p_canvas.rect( sx + 10.0f, sy + 8.0f, kSlot - 20.0f, kSlot - 24.0f, glm::vec4( 0.55f, 0.45f, 0.3f, 1.0f ) );

            if ( m_font == nullptr ) continue;

            char amount[ 16 ];
            std::snprintf( amount, sizeof( amount ), "%d", invSlot.m_amount );
            p_canvas.text( *m_font, amount, sx + kSlot - 22.0f, sy + kSlot - 22.0f, 14.0f, glm::vec4( 1.0f, 0.95f, 0.8f, 1.0f ) );

            if ( const Item* it = items.get( invSlot.m_itemId ); it != nullptr && !it->m_name.empty() )
            {
                const char letter[ 2 ] = { it->m_name.front(), '\0' };
                p_canvas.text( *m_font, letter, sx + 14.0f, sy + 12.0f, 18.0f, glm::vec4( 1.0f ) );
            }
        }
    }

    void UiLayer::drawClock( Tomos::TCanvas& p_canvas, const WorldClock& p_clock )
    {
        constexpr float kSize = 112.0f;
        const float     x     = p_canvas.width() - kSize - 20.0f;
        const float     y     = 16.0f;

        if ( m_panel != nullptr ) p_canvas.nineSlice( m_panel.get(), x, y, kSize, kSize, kMargins );

        const ClockSegs segs  = GameCalendar::segsFor( p_clock.m_season );
        const DayPhase  phase = p_clock.m_phase;
        const float     pad   = 16.0f;
        const float     barX  = x + pad;
        const float     barY  = y + pad + 8.0f;
        const float     barW  = kSize - pad * 2.0f;
        const float     barH  = 14.0f;
        const float     inv   = 1.0f / static_cast<float>( GameCalendar::g_kSegsPerDay );

        const float dayW   = barW * static_cast<float>( segs.m_day ) * inv;
        const float duskW  = barW * static_cast<float>( segs.m_dusk ) * inv;
        const float nightW = barW * static_cast<float>( segs.m_night ) * inv;

        p_canvas.rect( barX, barY, dayW, barH, glm::vec4( 0.95f, 0.82f, 0.25f, 1.0f ) );
        p_canvas.rect( barX + dayW, barY, duskW, barH, glm::vec4( 0.85f, 0.28f, 0.18f, 1.0f ) );
        p_canvas.rect( barX + dayW + duskW, barY, nightW, barH, glm::vec4( 0.22f, 0.28f, 0.55f, 1.0f ) );

        const float needleX = barX + barW * std::clamp( p_clock.m_timeOfDay, 0.0f, 1.0f );
        p_canvas.rect( needleX - 1.0f, barY - 3.0f, 2.0f, barH + 6.0f, glm::vec4( 0.12f, 0.08f, 0.04f, 1.0f ) );

        if ( m_font == nullptr ) return;

        char line0[ 48 ];
        char line1[ 48 ];
        std::snprintf( line0, sizeof( line0 ), "Day %d  Y%d", p_clock.m_dayCount, p_clock.m_year );
        std::snprintf( line1, sizeof( line1 ), "%s  %s", GameCalendar::seasonName( p_clock.m_season ).data(), GameCalendar::phaseName( phase ).data() );

        const glm::vec4 ink( 0.15f, 0.1f, 0.05f, 1.0f );
        const float     line0W = m_font->measure( line0, 16.0f );
        const float     line1W = m_font->measure( line1, 14.0f );
        p_canvas.text( *m_font, line0, x + ( kSize - line0W ) * 0.5f, y + kSize - 44.0f, 16.0f, ink );
        p_canvas.text( *m_font, line1, x + ( kSize - line1W ) * 0.5f, y + kSize - 26.0f, 14.0f, ink );
    }

    void UiLayer::onUi( float /*p_dt*/ )
    {
        Tomos::TCanvas& canvas = canvasBackend().canvas();
        if ( !canvas.recording() ) return;

        const auto* game = dynamic_cast<const InGameScene*>( &Tomos::TSceneLayer::scene() );
        if ( game == nullptr ) return;

        if ( const WorldClock* clock = game->clock() ) drawClock( canvas, *clock );

        const Survival* survival = game->survival();
        if ( survival == nullptr ) return;

        constexpr float kMeterW = 180.0f;
        constexpr float kMeterH = 48.0f;
        constexpr float kGap    = 8.0f;
        const float     baseX   = 16.0f;
        const float     baseY   = canvas.height() - ( kMeterH * 3.0f + kGap * 2.0f ) - 16.0f;

        drawMeter( canvas, baseX, baseY, kMeterW, kMeterH, statPct( survival->m_health ), glm::vec4( 0.75f, 0.2f, 0.2f, 1.0f ), "HP" );
        drawMeter( canvas, baseX, baseY + kMeterH + kGap, kMeterW, kMeterH, statPct( survival->m_hunger ), glm::vec4( 0.85f, 0.65f, 0.2f, 1.0f ), "HN" );
        drawMeter( canvas, baseX, baseY + ( kMeterH + kGap ) * 2.0f, kMeterW, kMeterH, statPct( survival->m_sanity ), glm::vec4( 0.35f, 0.45f, 0.85f, 1.0f ),
                   "SN" );
        drawInventory( canvas, *survival );
    }
}  // namespace NocInDan

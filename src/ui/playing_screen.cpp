#include "../app.h"

void App::render_playing() {
    float w = (float)renderer_.width();
    float h = (float)renderer_.height();

    // On webOS, the Luna media pipeline renders video to the screen directly.
    // We just show an overlay with minimal info.
    // On desktop, we show a placeholder.

#ifndef WEBOS_PLATFORM
    // Desktop placeholder
    renderer_.draw_rect({0, 0, w, h}, {0, 0, 0, 1});
    text_.draw_text(renderer_, "Playing (desktop preview)",
        w / 2, h / 2 - 30, ui::FontSize::Large,
        ui::colors::text_secondary, true);
    text_.draw_text(renderer_, detail_item_.display_title(),
        w / 2, h / 2 + 10, ui::FontSize::Title,
        ui::colors::white, true);
#endif

    // Playback status overlay (bottom)
    auto state = player_.state();
    std::string status;
    switch (state) {
        case media::PlayerState::Playing: status = "▶ Playing"; break;
        case media::PlayerState::Paused:  status = "⏸ Paused"; break;
        case media::PlayerState::Loading: status = "Loading..."; break;
        default: status = "Stopped"; break;
    }

    float bar_h = 50;
    renderer_.draw_rect({0, h - bar_h, w, bar_h}, {0, 0, 0, 0.75f});
    text_.draw_text(renderer_, status,
        20, h - bar_h + 14, ui::FontSize::Normal,
        ui::colors::text_primary);
    text_.draw_text(renderer_, detail_item_.display_title(),
        w / 2, h - bar_h + 14, ui::FontSize::Normal,
        ui::colors::text_secondary, true);
    text_.draw_text(renderer_, "BACK to stop",
        w - 20 - text_.text_width("BACK to stop", ui::FontSize::Small),
        h - bar_h + 16, ui::FontSize::Small,
        ui::colors::text_dim);
}

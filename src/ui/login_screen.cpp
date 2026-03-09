#include "../app.h"

void App::render_login() {
    float cx = renderer_.width() / 2.0f;
    float cy = renderer_.height() / 2.0f;

    // Dark card background
    float card_w = 400, card_h = 320;
    renderer_.draw_rect(
        {cx - card_w / 2, cy - card_h / 2, card_w, card_h},
        ui::Color::hex(0x2A2A2A)
    );

    // Title
    text_.draw_text(renderer_, "plex",
        cx, cy - card_h / 2 + 30, ui::FontSize::Title,
        ui::colors::gold, true);

    // Instructions
    text_.draw_text(renderer_, "Go to plex.tv/link",
        cx, cy - 40, ui::FontSize::Normal,
        ui::colors::text_secondary, true);
    text_.draw_text(renderer_, "and enter the code:",
        cx, cy - 10, ui::FontSize::Normal,
        ui::colors::text_secondary, true);

    // PIN code - space out characters for readability
    if (!current_pin_.code.empty()) {
        std::string spaced;
        for (size_t i = 0; i < current_pin_.code.size(); ++i) {
            if (i > 0) spaced += "  ";
            spaced += (char)toupper(current_pin_.code[i]);
        }
        text_.draw_text(renderer_, spaced,
            cx, cy + 40, ui::FontSize::Huge,
            ui::colors::white, true);
    } else {
        text_.draw_text(renderer_, "Requesting PIN...",
            cx, cy + 50, ui::FontSize::Normal,
            ui::colors::text_dim, true);
    }

    // Waiting indicator
    text_.draw_text(renderer_, "Waiting for authentication...",
        cx, cy + card_h / 2 - 50, ui::FontSize::Small,
        ui::colors::text_dim, true);
}

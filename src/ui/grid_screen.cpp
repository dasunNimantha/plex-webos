#include "../app.h"
#include <algorithm>

void App::render_grid() {
    float w = (float)renderer_.width();
    float h = (float)renderer_.height();

    // Header
    renderer_.draw_rect({0, 0, w, 50}, ui::colors::header_bg);
    text_.draw_text(renderer_, current_library_title_,
        w / 2, 12, ui::FontSize::Large, ui::colors::text_primary, true);

    // Grid layout
    int cols = 7;
    float card_w = 140;
    float card_h = 210;
    float spacing_x = 16;
    float spacing_y = 24;
    float grid_w = cols * (card_w + spacing_x) - spacing_x;
    float start_x = (w - grid_w) / 2;
    float start_y = 70;

    int total = (int)grid_items_.size();
    int rows = (total + cols - 1) / cols;

    // Scroll to keep focused row visible
    float row_h = card_h + spacing_y + 26; // 26 for title text
    float visible_h = h - start_y;
    float needed_y = focus_.row * row_h;

    if (needed_y - scroll_y_ > visible_h - row_h) {
        scroll_y_ = needed_y - visible_h + row_h;
    }
    if (needed_y < scroll_y_) {
        scroll_y_ = needed_y;
    }

    for (int r = 0; r < rows; ++r) {
        float y = start_y + r * row_h - scroll_y_;
        if (y + row_h < 50 || y > h) continue; // Off screen

        for (int c = 0; c < cols; ++c) {
            int idx = r * cols + c;
            if (idx >= total) break;

            auto& item = grid_items_[idx];
            float x = start_x + c * (card_w + spacing_x);

            // Focus highlight
            bool focused = (focus_.row == r && focus_.col == c);
            if (focused) {
                renderer_.draw_rect_outline(
                    {x - 3, y - 3, card_w + 6, card_h + 6},
                    ui::colors::gold, 3);
            }

            // Poster
            GLuint tex = 0;
            if (!item.thumb.empty()) {
                auto url = plex_.poster_url(item.thumb, 200, 300);
                tex = images_.get_texture(url);
                if (!tex) images_.request_image(url);
            }

            if (tex) {
                renderer_.draw_texture(tex, {x, y, card_w, card_h});
            } else {
                renderer_.draw_rect({x, y, card_w, card_h},
                    ui::Color::hex(0x2A2A2A));
            }

            // Title
            text_.draw_text(renderer_, item.display_title(),
                x, y + card_h + 4, ui::FontSize::Small,
                ui::colors::text_primary);

            // Subtitle (year)
            if (item.year > 0) {
                text_.draw_text(renderer_, std::to_string(item.year),
                    x, y + card_h + 20, ui::FontSize::Small,
                    ui::colors::text_dim);
            }
        }
    }

    // Update focus bounds
    focus_.max_cols = cols;
    focus_.max_rows = rows;

    // Clamp col for last row
    int last_row_items = total - focus_.row * cols;
    if (focus_.col >= last_row_items) {
        focus_.col = std::max(0, last_row_items - 1);
    }
}

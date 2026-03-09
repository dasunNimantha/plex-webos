#include "../app.h"
#include <algorithm>

void App::render_server_select() {
    float cx = renderer_.width() / 2.0f;

    text_.draw_text(renderer_, "Select Server",
        cx, 40, ui::FontSize::Title,
        ui::colors::white, true);

    float y = 120;
    for (int i = 0; i < (int)servers_.size(); ++i) {
        bool focused = (i == server_focus_);
        float card_w = 500, card_h = 60;
        float card_x = cx - card_w / 2;

        auto bg = focused ? ui::colors::gold : ui::Color::hex(0x2A2A2A);
        auto fg = focused ? ui::Color::hex(0x111111) : ui::colors::text_primary;

        renderer_.draw_rect({card_x, y, card_w, card_h}, bg);
        text_.draw_text(renderer_, servers_[i].name,
            cx, y + 18, ui::FontSize::Normal, fg, true);

        y += card_h + 10;
    }
}

void App::render_home() {
    float w = (float)renderer_.width();
    float y = 20;
    float sidebar_w = 200;
    float content_x = sidebar_w;

    // Sidebar background
    renderer_.draw_rect({0, 0, sidebar_w, (float)renderer_.height()}, ui::colors::sidebar_bg);

    // Sidebar header - "plex" logo
    text_.draw_text(renderer_, "plex",
        sidebar_w / 2, 16, ui::FontSize::Large,
        ui::colors::gold, true);

    // Sidebar items - Home + Libraries
    float sy = 70;
    bool home_focused = (focus_.row < 0);

    // Home row
    {
        auto bg = home_focused ? ui::Color{1,1,1,0.06f} : ui::colors::transparent;
        renderer_.draw_rect({4, sy, sidebar_w - 8, 36}, bg);
        auto color = home_focused ? ui::colors::gold : ui::colors::text_secondary;
        text_.draw_text(renderer_, "Home", 40, sy + 8, ui::FontSize::Normal, color);
        sy += 40;
    }

    // Library rows
    for (int i = 0; i < (int)libraries_.size(); ++i) {
        bool focused = (focus_.row == i && focus_.col == 0 &&
                        screen_ == Screen::Home);
        auto bg = focused ? ui::Color{1,1,1,0.06f} : ui::colors::transparent;
        renderer_.draw_rect({4, sy, sidebar_w - 8, 36}, bg);

        // Colored icon box
        ui::Color icon_color;
        if (libraries_[i].type == "movie")
            icon_color = ui::Color::hex(0xCC7B19);
        else if (libraries_[i].type == "show")
            icon_color = ui::Color::hex(0x4E7AB5);
        else
            icon_color = ui::Color::hex(0x7F8C8D);

        renderer_.draw_rect({14, sy + 8, 20, 20}, icon_color);

        auto text_color = focused ? ui::colors::gold : ui::colors::text_secondary;
        text_.draw_text(renderer_, libraries_[i].title,
            44, sy + 8, ui::FontSize::Normal, text_color);
        sy += 40;
    }

    // Settings at bottom
    text_.draw_text(renderer_, "Settings",
        sidebar_w / 2, (float)renderer_.height() - 40,
        ui::FontSize::Small, ui::colors::text_dim, true);

    // Content area - Hubs
    float hub_y = y;
    int lib_count = (int)libraries_.size();

    for (int hub_idx = 0; hub_idx < (int)hubs_.size(); ++hub_idx) {
        auto& hub = hubs_[hub_idx];
        if (hub.items.empty()) continue;

        // Hub title
        text_.draw_text(renderer_, hub.title,
            content_x + 24, hub_y, ui::FontSize::Large,
            ui::colors::text_primary);
        hub_y += 36;

        bool is_landscape = hub.hub_identifier.find("continue") != std::string::npos
                         || hub.hub_identifier.find("recentlyViewed") != std::string::npos;

        float card_w = is_landscape ? 260.0f : 140.0f;
        float card_h = is_landscape ? 146.0f : 210.0f;
        float card_spacing = 12;
        float cards_x = content_x + 24;

        int focus_hub = focus_.row - lib_count;

        for (int i = 0; i < (int)hub.items.size() && i < 8; ++i) {
            auto& item = hub.items[i];
            float cx = cards_x + i * (card_w + card_spacing);

            if (cx + card_w > w) break; // Off screen

            // Card background
            bool focused = (focus_hub == hub_idx && focus_.col == i);
            if (focused) {
                renderer_.draw_rect_outline(
                    {cx - 3, hub_y - 3, card_w + 6, card_h + 6},
                    ui::colors::gold, 3);
            }

            // Poster/art texture
            std::string img_url;
            if (is_landscape && !item.art.empty()) {
                img_url = plex_.art_url(item.art, 400, 225);
            } else if (!item.thumb.empty()) {
                img_url = plex_.poster_url(item.thumb, 200, 300);
            }

            GLuint tex = 0;
            if (!img_url.empty()) {
                tex = images_.get_texture(img_url);
                if (!tex) images_.request_image(img_url);
            }

            if (tex) {
                renderer_.draw_texture(tex, {cx, hub_y, card_w, card_h});
            } else {
                renderer_.draw_rect({cx, hub_y, card_w, card_h},
                    ui::Color::hex(0x2A2A2A));
            }

            // Title under card
            text_.draw_text(renderer_, item.display_title(),
                cx, hub_y + card_h + 6, ui::FontSize::Small,
                ui::colors::text_primary);
        }

        // Update max_cols for focus navigation
        if (focus_hub == hub_idx) {
            focus_.max_cols = std::min((int)hub.items.size(), 8);
        }

        hub_y += card_h + 40;
    }

    focus_.max_rows = lib_count + (int)hubs_.size();
}

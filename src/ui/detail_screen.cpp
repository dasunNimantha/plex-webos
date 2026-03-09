#include "../app.h"

void App::render_detail() {
    float w = (float)renderer_.width();

    // Header
    renderer_.draw_rect({0, 0, w, 50}, ui::colors::header_bg);
    text_.draw_text(renderer_, detail_item_.display_title(),
        w / 2, 12, ui::FontSize::Large, ui::colors::text_primary, true);

    // Layout: poster left, info right
    float poster_w = 180;
    float poster_h = 270;
    float poster_x = 60;
    float poster_y = 80;
    float info_x = poster_x + poster_w + 30;
    float info_w = w - info_x - 60;

    // Poster
    GLuint poster_tex = 0;
    if (!detail_item_.thumb.empty()) {
        auto url = plex_.poster_url(detail_item_.thumb, 300, 450);
        poster_tex = images_.get_texture(url);
        if (!poster_tex) images_.request_image(url);
    }
    if (poster_tex) {
        renderer_.draw_texture(poster_tex, {poster_x, poster_y, poster_w, poster_h});
    } else {
        renderer_.draw_rect({poster_x, poster_y, poster_w, poster_h},
            ui::Color::hex(0x2A2A2A));
    }

    // Title
    float ty = poster_y;
    text_.draw_text(renderer_, detail_item_.display_title(),
        info_x, ty, ui::FontSize::Title, ui::colors::white);
    ty += 42;

    // Tagline or director
    if (!detail_item_.tagline.empty()) {
        text_.draw_text(renderer_, detail_item_.tagline,
            info_x, ty, ui::FontSize::Normal, ui::colors::text_dim);
        ty += 26;
    } else if (!detail_item_.director.empty() && !detail_item_.director[0].tag.empty()) {
        text_.draw_text(renderer_, "Directed by " + detail_item_.director[0].tag,
            info_x, ty, ui::FontSize::Normal, ui::colors::text_dim);
        ty += 26;
    }

    // Meta: year, duration, rating
    std::string meta;
    if (detail_item_.year > 0) {
        meta += std::to_string(detail_item_.year);
    }
    if (detail_item_.duration > 0) {
        int mins = (int)(detail_item_.duration / 60000);
        int hrs = mins / 60;
        mins %= 60;
        if (!meta.empty()) meta += "  ";
        if (hrs > 0) meta += std::to_string(hrs) + "h " + std::to_string(mins) + "m";
        else meta += std::to_string(mins) + "m";
    }
    if (!detail_item_.content_rating.empty()) {
        if (!meta.empty()) meta += "  ";
        meta += detail_item_.content_rating;
    }
    if (!meta.empty()) {
        text_.draw_text(renderer_, meta,
            info_x, ty, ui::FontSize::Normal, ui::colors::text_secondary);
        ty += 26;
    }

    // Genres
    if (!detail_item_.genre.empty()) {
        std::string genres;
        for (auto& g : detail_item_.genre) {
            if (!genres.empty()) genres += ", ";
            genres += g.tag;
        }
        text_.draw_text(renderer_, genres,
            info_x, ty, ui::FontSize::Normal, ui::colors::text_secondary);
        ty += 26;
    }

    // Rating
    double r = detail_item_.audience_rating > 0 ? detail_item_.audience_rating : detail_item_.rating;
    if (r > 0) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.1f / 10", r);
        text_.draw_text(renderer_, std::string("★ ") + buf,
            info_x, ty, ui::FontSize::Normal, ui::colors::gold);
        ty += 30;
    }

    // Play button
    ty += 10;
    float btn_w = 120, btn_h = 44;
    renderer_.draw_rect({info_x, ty, btn_w, btn_h}, ui::colors::gold);
    text_.draw_text(renderer_, "▶  Play",
        info_x + btn_w / 2, ty + 10, ui::FontSize::Normal,
        ui::Color::hex(0x111111), true);
    ty += btn_h + 20;

    // Summary
    if (!detail_item_.summary.empty()) {
        text_.draw_text_wrapped(renderer_, detail_item_.summary,
            info_x, ty, info_w, ui::FontSize::Normal,
            ui::colors::text_secondary);
        ty += 80;
    }

    // Media info
    if (!detail_item_.media.empty()) {
        auto& m = detail_item_.media[0];
        ty += 10;
        if (!m.video_resolution.empty()) {
            text_.draw_text(renderer_,
                "Video    " + m.video_resolution + "p (" + m.video_codec + ")",
                info_x, ty, ui::FontSize::Small, ui::colors::text_dim);
            ty += 22;
        }
        if (!m.audio_codec.empty()) {
            text_.draw_text(renderer_,
                "Audio    " + m.audio_codec,
                info_x, ty, ui::FontSize::Small, ui::colors::text_dim);
            ty += 22;
        }
    }

    // Cast
    if (!detail_item_.role.empty()) {
        ty += 10;
        text_.draw_text(renderer_, "Cast & Crew",
            info_x, ty, ui::FontSize::Large, ui::colors::text_primary);
        ty += 36;

        float cx = info_x;
        for (int i = 0; i < (int)detail_item_.role.size() && i < 8; ++i) {
            // Circle avatar placeholder
            renderer_.draw_rect({cx, ty, 50, 50}, ui::Color::hex(0x2A2A2A));

            // Name
            text_.draw_text(renderer_, detail_item_.role[i].tag,
                cx, ty + 56, ui::FontSize::Small, ui::colors::text_secondary);

            cx += 80;
            if (cx + 80 > w - 60) break;
        }
    }
}

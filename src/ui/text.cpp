#include "text.h"
#include <cstdio>

namespace ui {

bool TextRenderer::init() {
    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        return false;
    }

    // Try common font paths
    const char* paths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
#ifdef WEBOS_PLATFORM
        "/usr/share/fonts/LGSmartUI-Regular.ttf",
        "/usr/share/fonts/LGSmartUI-Bold.ttf",
#endif
        nullptr
    };

    for (int i = 0; paths[i]; ++i) {
        TTF_Font* test = TTF_OpenFont(paths[i], 18);
        if (test) {
            TTF_CloseFont(test);
            font_path_ = paths[i];
            printf("Using font: %s\n", font_path_.c_str());
            return true;
        }
    }

    fprintf(stderr, "No suitable font found!\n");
    return false;
}

void TextRenderer::shutdown() {
    for (auto& [sz, font] : fonts_) {
        TTF_CloseFont(font);
    }
    fonts_.clear();
    TTF_Quit();
}

TTF_Font* TextRenderer::get_font(FontSize size) {
    int s = static_cast<int>(size);
    auto it = fonts_.find(s);
    if (it != fonts_.end()) return it->second;

    TTF_Font* font = TTF_OpenFont(font_path_.c_str(), s);
    if (!font) {
        fprintf(stderr, "Failed to open font at size %d: %s\n", s, TTF_GetError());
        return nullptr;
    }
    fonts_[s] = font;
    return font;
}

TextTexture TextRenderer::render_text(Renderer& ren, const std::string& text,
                                       FontSize size, const Color& color) {
    TTF_Font* font = get_font(size);
    if (!font || text.empty()) return {};

    SDL_Color sdl_color = {
        (Uint8)(color.r * 255), (Uint8)(color.g * 255),
        (Uint8)(color.b * 255), (Uint8)(color.a * 255)
    };

    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), sdl_color);
    if (!surface) return {};

    SDL_Surface* rgba = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface);
    if (!rgba) return {};

    TextTexture tt;
    tt.w = rgba->w;
    tt.h = rgba->h;
    tt.tex = ren.create_texture((unsigned char*)rgba->pixels, rgba->w, rgba->h, 4);
    SDL_FreeSurface(rgba);
    return tt;
}

void TextRenderer::draw_text(Renderer& ren, const std::string& text,
                              float x, float y, FontSize size,
                              const Color& color, bool center_x) {
    auto tt = render_text(ren, text, size, color);
    if (!tt.tex) return;

    float dx = center_x ? x - tt.w * 0.5f : x;
    ren.draw_texture(tt.tex, {dx, y, (float)tt.w, (float)tt.h});
    ren.delete_texture(tt.tex);
}

void TextRenderer::draw_text_wrapped(Renderer& ren, const std::string& text,
                                      float x, float y, float max_width,
                                      FontSize size, const Color& color) {
    TTF_Font* font = get_font(size);
    if (!font || text.empty()) return;

    SDL_Color sdl_color = {
        (Uint8)(color.r * 255), (Uint8)(color.g * 255),
        (Uint8)(color.b * 255), (Uint8)(color.a * 255)
    };

    SDL_Surface* surface = TTF_RenderUTF8_Blended_Wrapped(font, text.c_str(),
                                                           sdl_color, (Uint32)max_width);
    if (!surface) return;

    SDL_Surface* rgba = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface);
    if (!rgba) return;

    GLuint tex = ren.create_texture((unsigned char*)rgba->pixels, rgba->w, rgba->h, 4);
    ren.draw_texture(tex, {x, y, (float)rgba->w, (float)rgba->h});
    ren.delete_texture(tex);
    SDL_FreeSurface(rgba);
}

int TextRenderer::text_width(const std::string& text, FontSize size) {
    TTF_Font* font = get_font(size);
    if (!font) return 0;
    int w = 0, h = 0;
    TTF_SizeUTF8(font, text.c_str(), &w, &h);
    return w;
}

} // namespace ui

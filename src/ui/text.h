#pragma once

#include <SDL2/SDL_ttf.h>
#include <string>
#include <unordered_map>
#include "renderer.h"

namespace ui {

enum class FontSize {
    Small = 14,
    Normal = 18,
    Large = 24,
    Title = 32,
    Huge = 48,
    PinCode = 72,
};

struct TextTexture {
    GLuint tex = 0;
    int w = 0, h = 0;
};

class TextRenderer {
public:
    bool init();
    void shutdown();

    TextTexture render_text(Renderer& ren, const std::string& text,
                            FontSize size, const Color& color);

    void draw_text(Renderer& ren, const std::string& text,
                   float x, float y, FontSize size, const Color& color,
                   bool center_x = false);

    void draw_text_wrapped(Renderer& ren, const std::string& text,
                           float x, float y, float max_width,
                           FontSize size, const Color& color);

    int text_width(const std::string& text, FontSize size);

private:
    TTF_Font* get_font(FontSize size);
    std::unordered_map<int, TTF_Font*> fonts_;
    std::string font_path_;
};

} // namespace ui

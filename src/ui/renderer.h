#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <cstdint>

#ifdef WEBOS_PLATFORM
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>
#endif

namespace ui {

struct Color {
    float r, g, b, a;
    static Color hex(uint32_t hex, float alpha = 1.0f) {
        return {
            ((hex >> 16) & 0xFF) / 255.0f,
            ((hex >> 8) & 0xFF) / 255.0f,
            (hex & 0xFF) / 255.0f,
            alpha
        };
    }
};

struct Rect {
    float x, y, w, h;
};

class Renderer {
public:
    bool init(int width, int height, const std::string& title);
    void shutdown();

    void begin_frame();
    void end_frame();

    void clear(const Color& color);
    void draw_rect(const Rect& rect, const Color& color);
    void draw_rect_outline(const Rect& rect, const Color& color, float thickness = 2.0f);
    void draw_texture(GLuint tex, const Rect& dst);
    void draw_texture_rounded(GLuint tex, const Rect& dst, float radius);

    GLuint create_texture(const unsigned char* pixels, int w, int h, int channels);
    void delete_texture(GLuint tex);

    int width() const { return width_; }
    int height() const { return height_; }

    SDL_Window* window() { return window_; }

private:
    void init_shaders();
    void init_quad_vao();

    SDL_Window* window_ = nullptr;
    SDL_GLContext gl_ctx_ = nullptr;
    int width_ = 0, height_ = 0;

    GLuint shader_color_ = 0;
    GLuint shader_tex_ = 0;
    GLuint vbo_ = 0;
};

// Plex color constants
namespace colors {
    static const Color background = Color::hex(0x1F1F1F);
    static const Color sidebar_bg = Color::hex(0x1A1A1A);
    static const Color header_bg = Color::hex(0x1A1A1A);
    static const Color gold = Color::hex(0xE5A00D);
    static const Color white = {1, 1, 1, 1};
    static const Color text_primary = {1, 1, 1, 0.85f};
    static const Color text_secondary = {1, 1, 1, 0.50f};
    static const Color text_dim = {1, 1, 1, 0.35f};
    static const Color card_hover = {1, 1, 1, 0.05f};
    static const Color divider = {1, 1, 1, 0.04f};
    static const Color transparent = {0, 0, 0, 0};
}

} // namespace ui

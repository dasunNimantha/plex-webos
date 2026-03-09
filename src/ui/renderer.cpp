#include "renderer.h"
#include <cstdio>
#include <cstring>

namespace ui {

static const char* vert_src = R"(
#ifdef GL_ES
precision mediump float;
#endif
attribute vec2 a_pos;
attribute vec2 a_uv;
varying vec2 v_uv;
uniform vec4 u_rect;
uniform vec2 u_screen;
void main() {
    vec2 p = u_rect.xy + a_pos * u_rect.zw;
    vec2 ndc = (p / u_screen) * 2.0 - 1.0;
    ndc.y = -ndc.y;
    gl_Position = vec4(ndc, 0.0, 1.0);
    v_uv = a_uv;
}
)";

static const char* frag_color_src = R"(
#ifdef GL_ES
precision mediump float;
#endif
uniform vec4 u_color;
void main() {
    gl_FragColor = u_color;
}
)";

static const char* frag_tex_src = R"(
#ifdef GL_ES
precision mediump float;
#endif
varying vec2 v_uv;
uniform sampler2D u_tex;
void main() {
    gl_FragColor = texture2D(u_tex, v_uv);
}
)";

static GLuint compile_shader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(s, 512, nullptr, log);
        fprintf(stderr, "Shader error: %s\n", log);
    }
    return s;
}

static GLuint link_program(const char* vs, const char* fs) {
    GLuint v = compile_shader(GL_VERTEX_SHADER, vs);
    GLuint f = compile_shader(GL_FRAGMENT_SHADER, fs);
    GLuint p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glBindAttribLocation(p, 0, "a_pos");
    glBindAttribLocation(p, 1, "a_uv");
    glLinkProgram(p);
    glDeleteShader(v);
    glDeleteShader(f);
    return p;
}

bool Renderer::init(int width, int height, const std::string& title) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

#ifdef WEBOS_PLATFORM
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
#ifdef WEBOS_PLATFORM
    flags |= SDL_WINDOW_FULLSCREEN;
#endif

    window_ = SDL_CreateWindow(title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height, flags);
    if (!window_) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    gl_ctx_ = SDL_GL_CreateContext(window_);
    if (!gl_ctx_) {
        fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        return false;
    }
    SDL_GL_SetSwapInterval(1);

    SDL_GL_GetDrawableSize(window_, &width_, &height_);

    init_shaders();
    init_quad_vao();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    printf("Renderer init: %dx%d, GL: %s\n", width_, height_,
        (const char*)glGetString(GL_VERSION));
    return true;
}

void Renderer::init_shaders() {
    shader_color_ = link_program(vert_src, frag_color_src);
    shader_tex_ = link_program(vert_src, frag_tex_src);
}

void Renderer::init_quad_vao() {
    float verts[] = {
        // pos      uv
        0, 0,   0, 0,
        1, 0,   1, 0,
        0, 1,   0, 1,
        1, 1,   1, 1,
    };
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
}

void Renderer::shutdown() {
    if (shader_color_) glDeleteProgram(shader_color_);
    if (shader_tex_) glDeleteProgram(shader_tex_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (gl_ctx_) SDL_GL_DeleteContext(gl_ctx_);
    if (window_) SDL_DestroyWindow(window_);
    SDL_Quit();
}

void Renderer::begin_frame() {
    glViewport(0, 0, width_, height_);
}

void Renderer::end_frame() {
    SDL_GL_SwapWindow(window_);
}

void Renderer::clear(const Color& c) {
    glClearColor(c.r, c.g, c.b, c.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::draw_rect(const Rect& rect, const Color& color) {
    glUseProgram(shader_color_);
    glUniform4f(glGetUniformLocation(shader_color_, "u_rect"),
                rect.x, rect.y, rect.w, rect.h);
    glUniform2f(glGetUniformLocation(shader_color_, "u_screen"),
                (float)width_, (float)height_);
    glUniform4f(glGetUniformLocation(shader_color_, "u_color"),
                color.r, color.g, color.b, color.a);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(0);
}

void Renderer::draw_rect_outline(const Rect& rect, const Color& color, float t) {
    draw_rect({rect.x, rect.y, rect.w, t}, color);                    // top
    draw_rect({rect.x, rect.y + rect.h - t, rect.w, t}, color);      // bottom
    draw_rect({rect.x, rect.y, t, rect.h}, color);                    // left
    draw_rect({rect.x + rect.w - t, rect.y, t, rect.h}, color);      // right
}

void Renderer::draw_texture(GLuint tex, const Rect& dst) {
    glUseProgram(shader_tex_);
    glUniform4f(glGetUniformLocation(shader_tex_, "u_rect"),
                dst.x, dst.y, dst.w, dst.h);
    glUniform2f(glGetUniformLocation(shader_tex_, "u_screen"),
                (float)width_, (float)height_);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(glGetUniformLocation(shader_tex_, "u_tex"), 0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void Renderer::draw_texture_rounded(GLuint tex, const Rect& dst, float /*radius*/) {
    draw_texture(tex, dst);
}

GLuint Renderer::create_texture(const unsigned char* pixels, int w, int h, int channels) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    GLenum fmt = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return tex;
}

void Renderer::delete_texture(GLuint tex) {
    glDeleteTextures(1, &tex);
}

} // namespace ui

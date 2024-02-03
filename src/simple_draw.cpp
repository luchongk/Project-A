#include "rect.h"
#include "graphics.h"
#include "array.h"

static bool sd_initted = false;
static ShaderProgram*   sd_current_shader;   //@Cleanup: Already in graphics.cpp
static Texture*         sd_current_texture;  //@Cleanup: Already in graphics.cpp
static ShaderProgram*   sd_main_shader;
static ShaderProgram*   sd_text_shader;
static GraphicsBuffer*  sd_vertex_buffer;
static Array<VertexPCU> sd_vertices;

static u8 font_bitmap[512*512];
static stbtt_bakedchar font_glyphs[96];
Texture* texture_sd_font;

Texture* texture_panel;

static void sd_init() {
    sd_vertex_buffer = create_vertex_buffer(GraphicsBufferUsage::DYNAMIC, VERTEX_FORMAT_PCU, 2048);
    sd_main_shader   = compile_shader("assets\\shaders\\ui_vertex.hlsl"_s, "assets\\shaders\\ui_pixel.hlsl"_s, VERTEX_FORMAT_PCU);

    texture_panel = create_texture_from_file("assets\\textures\\panel.png"_s);

    texture_sd_font = load_font_file("assets\\fonts\\Inconsolata.ttf"_s, font_bitmap, font_glyphs);
    sd_text_shader  = compile_shader("assets\\shaders\\ui_vertex.hlsl"_s, "assets\\shaders\\text_pixel.hlsl"_s, VERTEX_FORMAT_PCU);

    sd_initted = true;
    sd_current_shader = nullptr;
}

void sd_flush() {
    if(sd_vertices.count == 0) return;

    set_depth(false);
    set_blend(true);
    
    set_vertex_buffer(sd_vertex_buffer);
    modify_buffer(sd_vertex_buffer, sizeof(VertexPCU) * sd_vertices.count, sd_vertices.data);
    draw(0, sd_vertices.count);
    
    array_reset(&sd_vertices);
    sd_current_shader = nullptr;
}

void set_shader_for_shapes(Texture* texture) {
    if(!sd_initted) sd_init();

    if(sd_current_shader != sd_main_shader || sd_current_texture != texture) {
        sd_flush();
        set_shader(sd_main_shader);
        sd_current_shader = sd_main_shader;
        set_texture(0, texture);
        sd_current_texture = texture;
    }
}

static void set_shader_for_text() {
    if(!sd_initted) sd_init();

    if(sd_current_shader != sd_text_shader || sd_current_texture != texture_sd_font) {
        sd_flush();
        set_shader(sd_text_shader);
        sd_current_shader = sd_text_shader;
        set_texture(0, texture_sd_font);
        sd_current_texture = texture_sd_font;
    }
}

void draw_triangle(Vec2 p1, Vec2 p2, Vec2 p3, Vec4 color) {
    set_shader_for_shapes(white_pixel);
    
    int screen_h = window->size.y;
    array_add(&sd_vertices, { Vec3{p1.x, screen_h - p1.y}, color });
    array_add(&sd_vertices, { Vec3{p2.x, screen_h - p2.y}, color });
    array_add(&sd_vertices, { Vec3{p3.x, screen_h - p3.y}, color });
}

void draw_quad(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec4 color) {
    set_shader_for_shapes(white_pixel);

    int screen_h = window->size.y;
    array_add(&sd_vertices, { Vec3{p1.x, screen_h - p1.y}, color });
    array_add(&sd_vertices, { Vec3{p2.x, screen_h - p2.y}, color });
    array_add(&sd_vertices, { Vec3{p3.x, screen_h - p3.y}, color });
    array_add(&sd_vertices, { Vec3{p1.x, screen_h - p1.y}, color });
    array_add(&sd_vertices, { Vec3{p3.x, screen_h - p3.y}, color });
    array_add(&sd_vertices, { Vec3{p4.x, screen_h - p4.y}, color });
}

void draw_quad(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec2 uv1, Vec2 uv2, Vec2 uv3, Vec2 uv4, Vec4 color, Texture* texture = white_pixel) {
    set_shader_for_shapes(texture);
    
    int screen_h = window->size.y;
    array_add(&sd_vertices, { Vec3{p1.x, screen_h - p1.y}, color, uv1 });
    array_add(&sd_vertices, { Vec3{p2.x, screen_h - p2.y}, color, uv2 });
    array_add(&sd_vertices, { Vec3{p3.x, screen_h - p3.y}, color, uv3 });
    array_add(&sd_vertices, { Vec3{p1.x, screen_h - p1.y}, color, uv1 });
    array_add(&sd_vertices, { Vec3{p3.x, screen_h - p3.y}, color, uv3 });
    array_add(&sd_vertices, { Vec3{p4.x, screen_h - p4.y}, color, uv4 });
}

void draw_rect(Rect rect, Vec4 color, Texture* texture) {
    Vec2 p1 = {rect.x, rect.y};
    Vec2 p2 = {rect.x, rect.y + rect.h};
    Vec2 p3 = {rect.x + rect.w, rect.y + rect.h};
    Vec2 p4 = {rect.x + rect.w, rect.y};

    if(!texture) {
        draw_quad(p1, p2, p3, p4, color);
    }
    else {
        draw_quad(p1, p2, p3, p4, Vec2{0,0}, Vec2{0,1}, Vec2{1,1}, Vec2{1,0}, color, texture);
    }
}

static float draw_char(char c, float x, float y, float scale, Vec4 color) {
    set_shader_for_text();

    float advance_x = 0;
    float advance_y = 0;

    stbtt_aligned_quad q;
    stbtt_GetBakedQuad(font_glyphs, 512, 512, c - 32, &advance_x, &advance_y, &q, 0);
    q.x0 = x + q.x0 * scale;
    q.x1 = x + q.x1 * scale;
    q.y0 = y - q.y0 * scale;
    q.y1 = y - q.y1 * scale;

    Vec2 p1 = {q.x0, q.y1};
    Vec2 p2 = {q.x1, q.y1};
    Vec2 p3 = {q.x1, q.y0};
    Vec2 p4 = {q.x0, q.y0};

    Vec2 uv1 = {q.s0, q.t1};
    Vec2 uv2 = {q.s1, q.t1};
    Vec2 uv3 = {q.s1, q.t0};
    Vec2 uv4 = {q.s0, q.t0};
    
    array_add(&sd_vertices, { Vec3{p1}, color, uv1 });
    array_add(&sd_vertices, { Vec3{p2}, color, uv2 });
    array_add(&sd_vertices, { Vec3{p3}, color, uv3 });
    array_add(&sd_vertices, { Vec3{p1}, color, uv1 });
    array_add(&sd_vertices, { Vec3{p3}, color, uv3 });
    array_add(&sd_vertices, { Vec3{p4}, color, uv4 });

    return advance_x * scale;
}

void draw_text(Vec2 origin, String text, float height, Vec4 color) {
    float scale = height / (ascent - descent);

    float x = origin.x;
    float y = origin.y - descent * scale;

    For(text) {        
        float advance = draw_char(*it, x, y, scale, color);
        x += advance;
    }
}

Vec4 darken(Vec4 color, float amount) {
    color.r *= (1 - amount);
    color.g *= (1 - amount);
    color.b *= (1 - amount);

    return color;
}

Vec4 lighten(Vec4 color, float amount) {
    color.r = color.r * (1 - amount) + amount;
    color.g = color.g * (1 - amount) + amount;
    color.b = color.b * (1 - amount) + amount;

    return color;
}
#include "rect.h"
#include "graphics.h"
#include "array.h"

static bool sd_initted = false;
static ShaderProgram* sd_current_shader;
static ShaderProgram* sd_main_shader;
static ShaderProgram* sd_text_shader;
static GraphicsBuffer*  sd_vertex_buffer;
static Array<VertexPCU> sd_vertices;

static u8 font_bitmap[512*512];
static stbtt_bakedchar font_glyphs[96];
Texture* sd_font_texture;

static void sd_init() {
    sd_vertex_buffer = create_vertex_buffer(GraphicsBufferUsage::DYNAMIC, VertexFormat::PCU, 1024);
    sd_main_shader   = compile_shader("assets\\shaders\\ui_vertex.hlsl"_s, "assets\\shaders\\ui_pixel.hlsl"_s, VertexFormat::PCU);

    sd_font_texture = load_font_file("assets\\fonts\\Inconsolata.ttf"_s, font_bitmap, font_glyphs);
    sd_text_shader  = compile_shader("assets\\shaders\\ui_vertex.hlsl"_s, "assets\\shaders\\text_pixel.hlsl"_s, VertexFormat::PCU);
    sd_initted = true;
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

static void set_shader_for_shapes() {
    if(!sd_initted) sd_init();

    if(sd_current_shader != sd_main_shader) {
        sd_flush();
        set_shader(sd_main_shader);
        set_texture(0, white_pixel);
        sd_current_shader = sd_main_shader;
    }
}

static void set_shader_for_text() {
    if(!sd_initted) sd_init();

    if(sd_current_shader != sd_text_shader) {
        sd_flush();
        set_shader(sd_text_shader);
        set_texture(0, sd_font_texture);
        sd_current_shader = sd_text_shader;
    }
}

static Vec2 to_normalized_coords(Vec2 p) {
    float x = p.x * 2 - 1;
    float y = (1 - p.y) * 2 - 1;
    
    return {x, y};
}

void draw_quad(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec4 color) {
    set_shader_for_shapes();

    p1 = to_normalized_coords(p1);
    p2 = to_normalized_coords(p2);
    p3 = to_normalized_coords(p3);
    p4 = to_normalized_coords(p4);
    
    array_add(&sd_vertices, { Vec3{p1.x, p1.y}, color });
    array_add(&sd_vertices, { Vec3{p2.x, p2.y}, color });
    array_add(&sd_vertices, { Vec3{p3.x, p3.y}, color });
    array_add(&sd_vertices, { Vec3{p1.x, p1.y}, color });
    array_add(&sd_vertices, { Vec3{p3.x, p3.y}, color });
    array_add(&sd_vertices, { Vec3{p4.x, p4.y}, color });
}

void draw_quad(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec2 uv1, Vec2 uv2, Vec2 uv3, Vec2 uv4, Vec4 color) {
    set_shader_for_shapes();

    p1 = to_normalized_coords(p1);
    p2 = to_normalized_coords(p2);
    p3 = to_normalized_coords(p3);
    p4 = to_normalized_coords(p4);
    
    array_add(&sd_vertices, { Vec3{p1.x, p1.y}, color, uv1 });
    array_add(&sd_vertices, { Vec3{p2.x, p2.y}, color, uv2 });
    array_add(&sd_vertices, { Vec3{p3.x, p3.y}, color, uv3 });
    array_add(&sd_vertices, { Vec3{p1.x, p1.y}, color, uv1 });
    array_add(&sd_vertices, { Vec3{p3.x, p3.y}, color, uv3 });
    array_add(&sd_vertices, { Vec3{p4.x, p4.y}, color, uv4 });
}

void draw_rect(Rect rect, Vec4 color) {
    set_shader_for_shapes();

    Vec2 p1 = {rect.x, rect.y};
    Vec2 p2 = {rect.x, rect.y + rect.height};
    Vec2 p3 = {rect.x + rect.width, rect.y + rect.height};
    Vec2 p4 = {rect.x + rect.width, rect.y};

    draw_quad(p1, p2, p3, p4, color);
}

static float draw_char(char c, float x, float y, float scale, Vec4 color) {
    set_shader_for_text();

    float advance_x = 0;
    float advance_y = 0;

    stbtt_aligned_quad q;
    stbtt_GetBakedQuad(font_glyphs, 512, 512, c - 32, &advance_x, &advance_y, &q, 1);
    q.x0 = x + q.x0 * scale / window->size.x;
    q.x1 = x + q.x1 * scale / window->size.x;
    q.y0 = y + q.y0 * scale / window->size.y;
    q.y1 = y + q.y1 * scale / window->size.y;

    Vec2 p1 = to_normalized_coords({q.x0, q.y1});
    Vec2 p2 = to_normalized_coords({q.x1, q.y1});
    Vec2 p3 = to_normalized_coords({q.x1, q.y0});
    Vec2 p4 = to_normalized_coords({q.x0, q.y0});

    Vec2 uv1 = {q.s0, q.t1};
    Vec2 uv2 = {q.s1, q.t1};
    Vec2 uv3 = {q.s1, q.t0};
    Vec2 uv4 = {q.s0, q.t0};
    
    array_add(&sd_vertices, { Vec3{p1.x, p1.y}, color, uv1 });
    array_add(&sd_vertices, { Vec3{p2.x, p2.y}, color, uv2 });
    array_add(&sd_vertices, { Vec3{p3.x, p3.y}, color, uv3 });
    array_add(&sd_vertices, { Vec3{p1.x, p1.y}, color, uv1 });
    array_add(&sd_vertices, { Vec3{p3.x, p3.y}, color, uv3 });
    array_add(&sd_vertices, { Vec3{p4.x, p4.y}, color, uv4 });

    return advance_x * scale;
}

void draw_text(String text, Vec2 origin, float height, Vec4 color) {
    float height_px = height * window->size.y;
    float scale = height_px / (ascent - descent);

    float x = origin.x;
    float y = origin.y + descent * scale / window->size.y;

    For(text) {        
        float advance = draw_char(*it, x, y, scale, color);
        x += advance / window->size.x;
    }
}

Vec4 darken(Vec4 color, float amount) {
    color.r *= (1 - amount);
    color.g *= (1 - amount);
    color.b *= (1 - amount);

    return color;
}
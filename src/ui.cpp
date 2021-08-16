#include "graphics.h"
#include "array.h"
#include "strings.h"
#include "vector.h"
#include "input.h"
#include "hashtable.h"

HashTable<int,UIButtonState> ui_button_states;
int ui_active_layer = 0;

uint ui_shader;
static GraphicsBuffer* ui_vertex_buffer;
static Array<VertexPC> ui_vertices;

Array<UIButtonState*> ui_buttons_to_draw;

static Vec2 to_normalized_coords(Vec2 p) {
    float x = p.x * 2 - 1;
    float y = (1 - p.y) * 2 - 1;
    
    return {x, y};
}

bool is_mouse_over(Rect rect) {
    Vec2 mouse = player_input.mouse_pos_normalized;
    
    if(mouse.x < rect.x || mouse.x > rect.x + rect.width ||
       mouse.y < rect.y || mouse.y > rect.y + rect.height)
    {
        return false;
    }

    return true;
}

void ui_init() {
    ui_shader = compile_shader("assets\\shaders\\ui_vertex.hlsl"_s, "assets\\shaders\\ui_pixel.hlsl"_s, VertexFormat::PC);
    
    ui_vertex_buffer = create_vertex_buffer(GraphicsBufferUsage::DYNAMIC, VertexFormat::PC, 1024, ui_vertices.data);
}

static void draw_quad(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec4 color) {
    p1 = to_normalized_coords(p1);
    p2 = to_normalized_coords(p2);
    p3 = to_normalized_coords(p3);
    p4 = to_normalized_coords(p4);
    
    array_add(&ui_vertices, { Vec3{p1.x, p1.y}, color });
    array_add(&ui_vertices, { Vec3{p3.x, p3.y}, color });
    array_add(&ui_vertices, { Vec3{p2.x, p2.y}, color });
    array_add(&ui_vertices, { Vec3{p1.x, p1.y}, color });
    array_add(&ui_vertices, { Vec3{p4.x, p4.y}, color });
    array_add(&ui_vertices, { Vec3{p3.x, p3.y}, color });
}

static void draw_rect(Rect rect, Vec4 color) {
    Vec2 p1 = {rect.x, rect.y};
    Vec2 p2 = {rect.x + rect.width, rect.y};
    Vec2 p3 = {rect.x + rect.width, rect.y + rect.height};
    Vec2 p4 = {rect.x, rect.y + rect.height};

    draw_quad(p1, p2, p3, p4, color);
}

static Vec4 darken(Vec4 color, float amount) {
    color.r *= (1 - amount);
    color.g *= (1 - amount);
    color.b *= (1 - amount);

    return color;
}

static void draw_ui_button(UIButtonState* state) {
    Vec4 color;

    if(state->pressed) {
        if(state->hovered) {
            color = darken(state->hover_color, 0.5f);
        }
        else {
            color = state->base_color;
        }
    }
    else if(state->hovered) {
        color = state->hover_color;
    }
    else {
        color = state->base_color;
    }
    
    draw_rect(state->rect, color);
}

static void ui_flush() {
    set_depth(false);
    set_blend(true);
    set_shader(ui_shader);
    set_vertex_buffer(ui_vertex_buffer);
    modify_buffer(ui_vertex_buffer, sizeof(VertexPC) * (uint)ui_vertices.count, ui_vertices.data);
    
    draw((uint)ui_vertices.count);
    set_blend(false);
    set_depth(true);
    
    array_reset(&ui_vertices);
}

void ui_render() {
    //player_input.mouse_interacting_with_ui = false;

    //Rect button_rect = {0.1f, 0.8f, 0.2f, 0.1f};
    Vec4 button_color = {0, 0, 0, 0.7f};
    Vec4 button_hover_color = {0.5f, 0, 0, 1.0f};
    
    /* auto state =  *///draw_ui_button(button_color, button_hover_color, 1);
    /* if(state->pressed) {
        for(int i = 0; i < entities_count; i++) {
            entities[i].material.specular.r = min(entities[i].material.specular.r + 1.0f * time.dt, 1.0f);
        }
    } */

    //button_rect.x += 0.25f;
//button_hover_color = {0, 0.5f, 0, 1.0f};
    
    /* state = */ //draw_ui_button(button_color, button_hover_color, 2);
    /* if(state->pressed) {
        for(int i = 0; i < entities_count; i++) {
            entities[i].material.specular.g = min(entities[i].material.specular.g + 1.0f * time.dt, 1.0f);
        }
    } */

    /*button_rect.x = player_input.mouse_pos_normalized.x - 0.1f;
    button_rect.y = player_input.mouse_pos_normalized.y - 0.05f;*/
    //button_rect.x += 0.25f;
//button_hover_color = {0, 0, 0.5f, 1.0f};
    
    /*state = *///draw_ui_button(button_color, button_hover_color, 3);
    /*if(state->pressed) {
        for(int i = 0; i < entities_count; i++) {
            entities[i].material.specular.b = min(entities[i].material.specular.b + 1.0f * time.dt, 1.0f);
        }
    }*/

    For(ui_buttons_to_draw) {
        draw_ui_button(*it);
    }

    ui_flush();
}

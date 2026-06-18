#include "font.h"
#include "ui.h"
#include "entities.h"
#include "general.h"

FontHandle inconsolata;
EntityHandle editor_selected;

void draw_text(Rect rect, ArrayView<GlyphInfo> glyphs, Vector4 color = {1,1,1,1}, int x_padding = 0) {
    float x = floorf(rect.x + x_padding);
    For(glyphs) {
        sd_draw_rect({x + it.x_offset, rect.y, it.subregion.w, it.subregion.h}, it.uv, it.atlas_texture, color);
        x += it.advance;
    }
}

ArrayView<GlyphInfo> ui_layout_text(FontHandle font, int size, String text, int x_padding = 0) {
    auto glyphs = get_glyph_run(font, size, text, temp_allocator);
    float width = 0;
    float height = glyphs[0].subregion.h;
    For(glyphs) width += it.advance;
    
    ui_element(); w_px(width + 2 * x_padding) h_px(height)
    return glyphs;
}

void ui_text(FontHandle font, int size, String text, Vector3 color = {1,1,1}, int x_padding = 0) {
    auto glyphs = ui_layout_text(font, size, text, x_padding);
    draw_text(current_element->rect, glyphs, color, x_padding);
}

void do_button(String text, FontHandle font = inconsolata, int font_size = 24, int padding = 8, Vector3 bg_color = {.1f,.1f,.1f}, Vector3 text_color = {1,1,1}) {
    auto glyphs = ui_layout_text(font, font_size, text, padding);

    auto i = ui_interactable(current_element->rect);
    auto color = lerp(bg_color, 2*bg_color, i->hover_t);
    color = lerp(color, 0.1f*bg_color, i->hold_t);
    sd_draw_rect(current_element->rect, color, 8, false);

    draw_text(current_element->rect, glyphs, text_color, padding);
}

void ui_text_field(char* data) {
    ui_element();
}

bool editor_on;
void ui_declare() {
    _row

    if(editor_on) {
        ui_enter_data_scope("devtools"_s); {
            auto e = ui_begin_element(); w_fit _pad_x(10) _pad_top(10) h_expand(1) _column _gap(5) {
                sd_draw_rect(e->rect, {0,0.2f,0.1f,0.95f});

                auto reset_count = ui_alloc<int>();
                auto reset_count_string = sprint("Reset count: %d", temp_allocator, *reset_count);
                ui_text(inconsolata, 18, reset_count_string);

                auto selected = get_entity(editor_selected);
                auto pos = selected ? selected->position : Vector3{};
                auto pos_string = sprint("Selected entity position: (%f, %f, %f)", temp_allocator, pos.x, pos.y, pos.z);
                ui_text(inconsolata, 18, pos_string);

                ui_begin_element(); h_fit _row _gap(15) _pad_top(10) {
                    do_button("Reset Scene!"_s);
                    if(on_click(current_interactable)) {
                        (*reset_count)++;
                        reset_scene();
                    }
                } ui_end_element();
            } ui_end_element();
        }
    }

    ui_begin_element(); w_expand(1) _column x_align(1) {
        auto fps_string = sprint("FPS: %.2f", temp_allocator, 1 / my_time.dt);
        ui_text(inconsolata, 18, fps_string);

        auto dt_string = sprint("dt: %.4fms", temp_allocator, my_time.dt);
        ui_text(inconsolata, 18, dt_string);
        
        auto time_scale = sprint("Time scale: %.2fx", temp_allocator, my_time.sim_scale);
        ui_text(inconsolata, 18, time_scale);
    } ui_end_element();

    auto size = get_window_size(the_window);
    sd_draw_rect({(float)size.x / 2 - 5, (float)size.y / 2 - 5, 10, 10}, {1,1,1,1});
}

void init_editor() {
    sd_init();
    ui_init(2560, 1440);
    inconsolata = font_load_from_file("assets\\fonts\\Inconsolata.ttf"_s);
}

void end_editor() {
    font_release(inconsolata);
    free_font_atlas();
}
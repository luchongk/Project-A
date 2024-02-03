#include "new_ui.h"

void ui_button(Rect rect, Vec4 color, EventHandler on_click, String name) {
    UIButton* state = get_or_create(&ui_buttons, name, {});
    state->rect     = rect;
    state->on_click = on_click;

    draw_rect(rect, color);
}

//@Journey 10/31/2023: Did some research on D3D vertex position precision. The rasterizer converts floating point positions to fixed point of size 16.8
// after applying the viewport transform, which means 8 bits are used for the fractional part of the value. That means pixels get divided into 256 subpixels.
// Furthermore the max error while converting to fixed point is apparently D3D11_FLOAT32_TO_INTEGER_TOLERANCE_IN_ULP (as documented here https://learn.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-data-conversion)
// which at the time of writing is 0.6. My guess is that this means 0.6 of a subpixel which would translate to plus minus 0.6/256 = 0.00234375 of a pixel of error.
// I corroborated this claim with a few manual tests and it seems to hold:

// On a 2560x1440 monitor:

// Pixel coordinate 0.5 should be between -0.99961120605 and -0.99960754394. Exact coordinate is -0.999609375
// It was at -0.999607742f.
// -0.999607742 - (-0.999609375) * 2560 / 2 = 0.00209024 < 0.00234375

// Pixel coordinate 1.5 should be between -0.99882995605 and -0.99882629394. Exact coordinate is -0.998828125
// It was at -0.998826504f.
// (-0.998826504 - (-0.998828125)) * 2560 / 2 = 0.00207488 < 0.00234375

int debug_values_count = 0;
void ui_build() {
    float screen_w = (float)window->size.x;
    float screen_h = (float)window->size.y;
    Rect screen = {0,0,screen_w,screen_h};
    Rect side_bar = cut_right(&screen, screen_w * 0.2f);
    draw_rect(side_bar, {0.522f, 0.514f, 0.62f, 0.8f});
    
    float h = screen_h * 0.2f;
    Rect r1 = cut_top(&side_bar, h);
    
    r1 = pad_left_right(r1, r1.w * 0.25f * (1 + cosf(my_time.since_start)));
    r1 = pad_top_bottom(r1, r1.h * 0.25f * (1 + cosf(my_time.since_start)));
    ui_button(r1, {1,0,0,1}, [](){}, "B1"_s);

    r1 = cut_top(&side_bar, h);
    ui_button(r1, {0,1,0,1}, [](){}, "B2"_s);

    r1 = cut_top(&side_bar, h);
    ui_button(r1, {0,0,1,1}, [](){}, "B3"_s);

    r1 = cut_top(&side_bar, h);
    ui_button(r1, {1,1,0,1}, [](){}, "B4"_s);

    ui_button(side_bar, {0,1,1,1}, [](){}, "B5"_s);

    draw_rect({5,0,600,debug_values_count * 32.0f + 5}, {0,0,0,0.5f});
    
    debug_values_count = 0;
    debug_value("main_camera.y", main_camera->entity->position.y);
    debug_value("main_camera.z", main_camera->entity->position.z);
    debug_value("player.x",      main_player->entity->position.x);
    debug_value("player.vel.x",  main_player->velocity.x);
    debug_value("player.vel.y",  main_player->velocity.y);
    debug_value("other_player.vel.x", player2->velocity.x);
    debug_value("other_player.vel.y", player2->velocity.y);

    sd_flush();
}

void debug_value(const char* name, float value) {
    char result[256];
    sprintf(result, "%s = %f", name, value);
    
    Vec2 text_pos = {10, window->size.y - (debug_values_count + 1) * 32.0f};
    debug_values_count++;
    draw_text(text_pos, from_cstring(result), 32.0f, {1,1,1,1});
}

void ui_input() {

}

void ui_update() {

}
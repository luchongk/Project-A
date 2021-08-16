#ifndef UI_H
#define UI_H

#include "hashtable.h"

struct Rect {
    float x;
    float y;
    float width;
    float height;
};

struct UIButtonState {
    Rect rect;
    Vec4 base_color = {0,0,0,1};
    Vec4 hover_color = {1,1,1,1};
    bool hovered;
    bool pressed;
    bool clicked;
};

void ui_init();
void ui_render();
bool is_mouse_over(Rect rect);

extern uint ui_shader;
extern HashTable<int,UIButtonState> ui_button_states;
extern Array<UIButtonState*> ui_buttons_to_draw;

#endif
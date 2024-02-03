#ifndef NEW_UI_H
#define NEW_UI_H

#include "rect.h"
#include "hashtable.h"

typedef void(*EventHandler)();

struct UIButton {
    Rect rect;
    EventHandler on_click;
};

HashTable<String,UIButton> ui_buttons;

void ui_build();
void ui_input();
void ui_update();
void debug_value(const char* name, float value);

#endif
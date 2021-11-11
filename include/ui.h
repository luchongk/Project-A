#ifndef UI_H
#define UI_H

#include "rect.h"
#include "hashtable.h"

enum class UIElementType {
    PANEL,
    BUTTON,
    SLIDER,
    TEXT_FIELD,
};

struct UIElement {
    String name;  //@Speed: Hash this
    Rect rect;
    UIElementType type;
    void* type_data;
};

enum class UIActionType {
    MOVE,
    SIZE,
};

struct UIActionMove{
    Vec2 pivot;
};

struct UIActionSize{
    Vec2 pivot;
};

struct UIAction {
    UIActionType type;
    UIElement* element;
    bool lock_hot = false;
    union {
        UIActionMove move;
        UIActionSize size;
    };
};

/*namespace UIButtonFlags {
    u8
    NONE    = 0,
    HOVERED = 1,
    PRESSED = 2,
    CLICKED = 4;
}*/

struct UIPanel {
    UIElement* element;
    bool visible = true;
    Vec4 base_color = {0,0,0,0.3f};
    uint children_count = 0;
    UIElement* children[16];
};

struct UIButton {
    UIElement* element;
    Vec4 base_color  = {1,0,0,1};
    Vec4 hover_color = {1,1,1,1};
    //u8 state = UIButtonFlags::NONE;
};

struct UISlider {
    UIElement* element;
    Vec4 base_color   = {0.0f,0.5f,0.5f,1};
    Vec4 button_color = {0.2f,0.2f,0.2f,1};
    float* value;
    float target_value;
};

struct UITextField {
    UIElement* element;
    char value[64];
    int count = 0;
};

void ui_init();
void ui_reset();
void ui_update_hot();
void ui_handle_text_event(EventText* event);
void ui_handle_click_event(EventKey* event);
void ui_update();
void ui_render();
bool is_mouse_over(Rect rect);

extern bool ui_visible;
extern UIAction ui_current_action;
// I don't think these should be dynamic, but lets have them be that for now.
extern Array<UIElement>    ui_elements;
extern Array<UIPanel>      ui_panels;
extern Array<UIButton>     ui_buttons;
extern Array<UISlider>     ui_sliders;
extern Array<UITextField>  ui_text_fields;
extern UIElement* ui_hot;
extern UIPanel*   ui_hot_panel;

#endif
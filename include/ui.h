#ifndef UI_H
#define UI_H

#include "rect.h"
#include "hashtable.h"

struct UIWidget;

ENUM(UIWidgetType,
    PANEL,
    BUTTON,
    SLIDER,
    TEXT,
    TEXT_FIELD,
);

ENUM(UIEventType,
    CLICK,
    TEXT
);

struct UIEventHandler {
    UIEventType type;
    union {
        bool (*key)(UIWidget* widget, EventKey* event);
    };
};

struct UIWidget {
    Rect rect;
    String name;  //@Speed: Hash this
    Array<UIEventHandler> handlers;
    UIWidgetType type;
    void* type_data;
    bool visible = true;
};

enum class UIActionType : u8 {
    MOVE,
    SIZE,
};

struct UIActionMove{
    Vec2 pivot;
};

struct UIActionSize{
    Vec2 pivot;
};

// In all UIs (that I know of), there is at most only 1 action being done by the user at any point in time.
// That means we can store the current action as a global that anyone can access.
struct UIAction {
    UIWidget* widget;
    String    widget_id;
    UIActionType type;        // Action type, interpreted by each widget however they need.
    u8 data[32];    // 32 bytes of custom data associated with the action.
};

/*namespace UIButtonFlags {
    u8
    NONE    = 0,
    HOVERED = 1,
    PRESSED = 2,
    CLICKED = 4;
}*/

struct UIPanel {
    UIWidget* widget;
    bool visible = true;
    Vec4 base_color = {0,0,0,0.3f};
    uint children_count = 0;
    UIWidget* children[16];
};

struct UIButton {
    UIWidget* widget;
    Vec4 base_color  = {1,0,0,1};
    Vec4 hover_color = {1,1,1,1};
    //u8 state = UIButtonFlags::NONE;
};

struct UISlider {
    UIWidget* widget;
    Vec4 base_color   = {0.0f,0.5f,0.5f,1};
    Vec4 button_color = {0.2f,0.2f,0.2f,1};
    float* value;
    float target_value;
};

struct UIText {
    UIWidget* widget;
    u8* text;
};

struct UITextField {
    UIWidget* widget;
    char value[64];
    int count = 0;
};

void ui_init();
void ui_begin();
void ui_update_hot();
bool ui_handle_click_event(EventKey* event);
bool ui_handle_key_event(EventKey* event);
bool ui_handle_text_event(EventText* event);
void ui_update();
void ui_render();
bool is_mouse_over(Rect rect);

extern bool ui_visible;
// I don't think these should be dynamic, but lets have them be that for now.
extern Array<UIWidget>    ui_widgets;
extern Array<UIPanel>     ui_panels;
extern Array<UIButton>    ui_buttons;
extern Array<UISlider>    ui_sliders;
extern Array<UITextField> ui_text_fields;
extern Array<UIText>      ui_texts;
extern UIWidget* ui_hot;
extern UIPanel*  ui_hot_panel;
extern UIAction  ui_current_action;
extern UIWidget* ui_focused;

#endif
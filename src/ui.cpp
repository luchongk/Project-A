#include "graphics.h"
#include "array.h"
#include "strings.h"
#include "vector.h"
#include "input.h"
#include "hashtable.h"

// Things to remember/think about:
// - If we want to implement tooltips we could have another widget function that takes in the widget and draws the tooltip for it.
//   The only thing that the ui system would do is call this function at the right time (ie. when the user hovers over for more than X amount of time or whatever).
//
// - How do we make it easy for developers to respond to clicks that happen outside certain widgets (click outside popup, for example)?
//   The web way would be to attach an 'on click' listener to the document and on each click check if the widget we are interested in is in the path from the root to the target of the click
//   I think the idea is ok, but we could certainly make it part of the API instead of having developers write it every time, and there may even be some optimizations we can do if we do that.
//
// - With the current implementation of actions its possible for the active widget to be aware of multiple modes, but what if we want a certain action to change the default behavior of clicks?
//   Does that even make sense for a functional UI? I think this kind of modal interactions should be implemented by the application and not the library, but we will see...


bool ui_visible = false;

Array<UIWidget>     ui_elements;
Array<UIPanel>      ui_panels;
Array<UIButton>     ui_buttons;
Array<UISlider>     ui_sliders;
Array<UITextField>  ui_text_fields;

UIWidget* ui_hot       = nullptr;
UIPanel*  ui_hot_panel = nullptr;
UIAction  ui_current_action;
UIWidget* ui_focused = nullptr;

Array<UIWidget*> updatables;

bool is_mouse_over(Rect rect) {
    Vec2 mouse = input.mouse_pos_normalized;
    
    if(mouse.x < rect.x || mouse.x > rect.x + rect.width ||
       mouse.y < rect.y || mouse.y > rect.y + rect.height)
    {
        return false;
    }

    return true;
}

static void ui_end_action() {
    ui_current_action.widget  = nullptr;
}

void ui_reset() {
    array_reset(&ui_elements);
    array_reset(&ui_buttons);
    array_reset(&ui_panels);
    array_reset(&ui_sliders);

    ui_init();
}

static UIWidget* ui_create_element(Rect rect, UIWidgetType type, String name) {
    UIWidget e;
    e.rect = rect;
    e.name = name;
    e.type = type;
    return array_add(&ui_elements, e);
}

UIPanel* ui_create_panel(Rect rect, String name) {
    UIWidget* widget = ui_create_element(rect, UIWidgetType::PANEL, name);
    
    UIPanel p{};
    p.widget = widget;
    auto panel = array_add(&ui_panels, p);;
    widget->type_data = panel;

    return panel;
}

static void add_to_panel(UIPanel* panel, UIWidget* widget) {
    panel->children[panel->children_count++] = widget;
}

UIButton* ui_create_button(Rect rect, String name) {
    UIWidget* widget = ui_create_element(rect, UIWidgetType::BUTTON, name);
    
    UIButton b{};
    b.widget = widget;
    auto button = array_add(&ui_buttons, b);
    widget->type_data = button;

    return button;
}

UISlider* ui_create_slider(Rect rect, String name, float* value = nullptr) {
    UIWidget* widget = ui_create_element(rect, UIWidgetType::SLIDER, name);

    UISlider s{};
    s.value = value;
    s.target_value = *value;
    s.widget = widget;
    auto slider = array_add(&ui_sliders, s);
    widget->type_data = slider;

    return slider;
}

UITextField* ui_create_text_field(Rect rect, String name) {
    UIWidget* widget = ui_create_element(rect, UIWidgetType::TEXT_FIELD, name);
    
    UITextField t{};
    t.widget = widget;
    auto field = array_add(&ui_text_fields, t);
    widget->type_data = field;

    return field;
}

static bool on_click_resume(UIWidget* widget, EventKey* event) {
    /*{
        // Hide UI.
        ui_visible = false;
        ui_current_action.widget = nullptr;
    }*/

    {
        widget->visible = false;
    }

    return true;
}

void ui_init() {
    array_reserve(&ui_elements, 64);
    ui_end_action();

    Rect rect = {0.01f, 0.01f, 0.2f, 0.4f};
    auto panel = ui_create_panel(rect, "MY_PANEL"_s);

    rect = pad_rect({0,0,1,1}, 0.1f);
    rect = cut_top(&rect, 0.2f);
    rect = pad_top_bottom(rect, 0.75f);
    auto slider = ui_create_slider(rect, "SLIDER_R"_s, &light->diffuse.r);
    slider->base_color = {1,0,0,1};
    add_to_panel(panel, slider->widget);

    rect.y += rect.height;
    slider = ui_create_slider(rect, "SLIDER_G"_s, &light->diffuse.g);
    slider->base_color = {0,1,0,1};
    add_to_panel(panel, slider->widget);

    rect.y += rect.height;
    slider = ui_create_slider(rect, "SLIDER_B"_s, &light->diffuse.b);
    slider->base_color = {0,0,1,1};
    add_to_panel(panel, slider->widget);

    rect.y += rect.height + 0.05f;
    slider = ui_create_slider(rect, "BG_SLIDER_R"_s, &background.r);
    slider->base_color = {1,0,0,1};
    add_to_panel(panel, slider->widget);

    rect.y += rect.height;
    slider = ui_create_slider(rect, "BG_SLIDER_G"_s, &background.g);
    slider->base_color = {0,1,0,1};
    add_to_panel(panel, slider->widget);

    rect.y += rect.height;
    slider = ui_create_slider(rect, "BG_SLIDER_B"_s, &background.b);
    slider->base_color = {0,0,1,1};
    add_to_panel(panel, slider->widget);

    rect.y += rect.height;
    auto button = ui_create_button(rect, "BUTTON_RESUME"_s);
    button->base_color = {0,0,0,1};
    array_add(&button->widget->handlers, {UIEventType::CLICK, on_click_resume});
    add_to_panel(panel, button->widget);

    rect.y += rect.height;
    auto field = ui_create_text_field(rect, "FIELD_PEPEGA"_s);
    add_to_panel(panel, field->widget);

    rect.y += rect.height * 2;
    field = ui_create_text_field(rect, "FIELD_PEPEGA2"_s);
    add_to_panel(panel, field->widget);
    
    ui_visible = true;
}

void ui_update_hot() {
    ui_hot = nullptr;
    
    For(ui_panels) {
        Rect rect = it->widget->rect;
        if(it->visible && is_mouse_over(rect)) {
            ui_hot_panel = it;
            
            for(uint i = 0; i < it->children_count; i++) {
                UIWidget* child = it->children[i];
                Rect child_rect = relative_to_screen(rect, child->rect);
                if(child->visible && is_mouse_over(child_rect)) {
                    ui_hot = child;
                    return;
                }
            }

            ui_hot = it->widget;
            return;
        }
    }
}

bool ui_handle_click_event(EventKey* event) {
    if(!ui_visible) return false;

    auto active = ui_current_action.widget;

    if(!active && !ui_hot) {
        ui_end_action();
        ui_focused = nullptr;
        return false;
    }

    // Using ui_hot here means that we don't allow real "in between frames" clicks, because we consider the click position to always be the the last position registered for the mouse in last frame, regardless of where the click really happened.
    // We do still register multiple clicks per frame if they happen, they just might not be exactly at the right position. I don't think we care about this right now, if at all, because the cases that make this implementation incorrect
    // would occur when clicking while moving the mouse REALLY fast, at which point you are just probably trolling with the UI instead of using it legitimately.
    // Doing it like this saves us from calculating the widget on which each click landed; we just assume all of them land on the hot widget, which is highly likely.
    if(event->pressed) {
        ui_current_action.widget = ui_hot;
        ui_focused = ui_hot;

        switch(ui_hot->type) {
            case UIWidgetType::SLIDER: {
                if(event->keycode == VK_LBUTTON) {
                    array_find_or_add(&updatables, ui_hot);
                }

                break;
            }

            case UIWidgetType::PANEL: {
                if(event->keycode == VK_LBUTTON) {
                    Rect rect = ui_hot->rect;
                    auto action = &ui_current_action;

                    action->widget = ui_hot;
                    action->type = UIActionType::MOVE;
                    auto move = (UIActionMove*)action->data;
                    move->pivot.x = input.mouse_pos_normalized.x - rect.x;
                    move->pivot.y = input.mouse_pos_normalized.y - rect.y;

                    array_find_or_add(&updatables, ui_hot);
                }
                else if(event->keycode == VK_RBUTTON) {
                    Rect rect = ui_hot->rect;
                    auto action = &ui_current_action;
                    
                    action->widget = ui_hot;
                    action->type = UIActionType::SIZE;
                    auto size = (UIActionSize*)action->data;
                    size->pivot.x = rect.width  - (input.mouse_pos_normalized.x - rect.x);
                    size->pivot.y = rect.height - (input.mouse_pos_normalized.y - rect.y);
                    
                    array_find_or_add(&updatables, ui_hot);
                }
                
                break;
            }
        }
    }
    else /* if(!event->pressed) */ {
        if(active == ui_hot) {
            For(active->handlers) {
                if(it->type == UIEventType::CLICK) return it->key(active, event);
            }
        }

        // We are making the assumption here, that a click outside the active widget ends the current action. This may or may not be true so we will have to see how this evolves.
        ui_end_action();
    }

    return true;
}

bool ui_handle_key_event(EventKey* event) {
    if(!ui_visible || !ui_focused) return false;

    auto widget = ui_focused;

    switch(widget->type) {
        case UIWidgetType::TEXT_FIELD: {
            auto field = (UITextField*)widget->type_data;
            auto keycode = event->keycode;
            
            if(event->pressed) {
                if(keycode == VK_BACK) {
                    if(field->count > 0) field->count--;
                }
            }

            return true;
        }
    }

    return false;
}

bool ui_handle_text_event(EventText* event) {
    if(!ui_focused) return false;

    auto widget = ui_focused;

    switch(widget->type) {
        case UIWidgetType::TEXT_FIELD: {
            auto field = (UITextField*)widget->type_data;

            if(field->count < 64) {
                field->value[field->count++] = event->character;
            }
            
            return true;
        }
    }

    return false;
}

void ui_update() {
    for(uint i = 0; i < updatables.count;) {
        auto it = updatables[i];

        auto keep_updating = true;  // @Cleanup: This will be the return value for update functions.

        switch(it->type) {
            case UIWidgetType::SLIDER: {
                auto slider = (UISlider*)it->type_data;
                
                if(ui_current_action.widget == it) {
                    auto panel_rect  = ui_hot_panel->widget->rect;
                    auto slider_rect = relative_to_screen(panel_rect, it->rect);
                    
                    auto slide_max       = slider_rect.width * 0.9f;
                    auto slide_distance  = input.mouse_pos_normalized.x - slider_rect.x - slider_rect.width * 0.05f;
                    slider->target_value = clamp(slide_distance / slide_max, 0.0f, 1.0f);
                }

                *slider->value = exp_interpolate(*slider->value, slider->target_value, my_time.dt, 20);
                if(*slider->value == slider->target_value && ui_current_action.widget != it) {
                    keep_updating = false;
                }

                break;
            }

            case UIWidgetType::PANEL: {
                if(ui_current_action.widget != it) {
                    keep_updating = false;
                }

                switch(ui_current_action.type) {
                    case UIActionType::MOVE: {
                        auto move = (UIActionMove*)ui_current_action.data;
                        auto pivot = move->pivot;
                        it->rect.x = input.mouse_pos_normalized.x - pivot.x;
                        it->rect.y = input.mouse_pos_normalized.y - pivot.y;
                        break;
                    }

                    case UIActionType::SIZE: {
                        auto size = (UIActionSize*)ui_current_action.data;
                        auto pivot = size->pivot;
                        it->rect.width  = input.mouse_pos_normalized.x - it->rect.x + pivot.x;
                        it->rect.height = input.mouse_pos_normalized.y - it->rect.y + pivot.y;
                        break;
                    }
                }

                break;
            }
        }

        if(!keep_updating) array_remove_by_index_unordered(&updatables, i);
        else ++i;
    }
}

static void ui_flush(ShaderProgram* shader) {
    set_depth(false);
    set_blend(true);
    set_shader(shader);
    set_vertex_buffer(sd_vertex_buffer);
    modify_buffer(sd_vertex_buffer, sizeof(VertexPCU) * sd_vertices.count, sd_vertices.data);
    
    draw(0, sd_vertices.count);
    set_blend(false);
    set_depth(true);
    
    array_reset(&sd_vertices);
}

static void core_draw_button(Rect rect, Vec4 base_color, Vec4 hover_color, bool hovered, bool pressed) {
    Vec4 color;
    if(pressed) {
        color = darken(hover_color, 0.5f);
    }
    else if(hovered) {
        color = hover_color;
    }
    else {
        color = base_color;
    }
    
    draw_rect(rect, color);
}

static void draw_button(UIWidget* widget, UIPanel* panel) {
    auto button = (UIButton*)widget->type_data;
    
    bool hovered = widget == ui_hot;
    bool pressed = widget == ui_current_action.widget;
    
    Rect rect = relative_to_screen(panel->widget->rect, widget->rect);
    core_draw_button(rect, button->base_color, button->hover_color, hovered, pressed);
}

static void draw_slider(UIWidget* widget, UIPanel* panel) {
    auto slider = (UISlider*)widget->type_data;

    auto slider_rect = relative_to_screen(panel->widget->rect, widget->rect);
    draw_rect(slider_rect, slider->base_color);

    Rect button = slider_rect;
    button.width *= 0.1f;
    button.x = slider_rect.x + (slider_rect.width - button.width) * *slider->value;
    bool pressed = ui_current_action.widget == widget;
    core_draw_button(button, slider->button_color, slider->button_color, true, pressed);
}

static void draw_text_field(UIWidget* widget, UIPanel* panel) {
    auto field = (UITextField*)widget->type_data;

    auto field_rect = relative_to_screen(panel->widget->rect, widget->rect);
    
    auto selected_rect = pad_rect(field_rect, -0.2f);
    if(ui_focused == widget) draw_rect(selected_rect, {0,0,1,1});
    draw_rect(field_rect, {0.3f, 0.3f, 0.3f, 1.0f});

    String text = from_cstring(field->value, field->count);
    /*
    char text_data[255];
    int count = 0;
    //if(ui_current_action.widget) {
        For(updatables) {
            for(uint i = 0; i< (*it)->name.count; i++) {
                text_data[count++] = (*it)->name.data[i];
            }
            
            text_data[count++] = ' ';
        }*/
        Vec2 origin = {field_rect.x, field_rect.y + field_rect.height};
        draw_text(text, origin, field_rect.height, {1,0,0,1});
    //}
}

static void draw_panel(UIPanel* panel) {
    draw_rect(panel->widget->rect, panel->base_color);
    
    for(uint i = 0; i < panel->children_count; i++) {
        auto child = panel->children[i];
        assert(child);
        if(!child->visible) continue;

        switch(child->type) {
            case UIWidgetType::BUTTON: {
                draw_button(child, panel);
                break;
            }

            case UIWidgetType::SLIDER: {
                draw_slider(child, panel);
                break;
            }

            case UIWidgetType::TEXT_FIELD: {
                draw_text_field(child, panel);
                break;
            }
        }
    }
}

void ui_render() {
    
    For(ui_panels) {
        if(it->visible) draw_panel(it);
    }

    sd_flush();
}

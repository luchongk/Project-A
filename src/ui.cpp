#include "graphics.h"
#include "array.h"
#include "strings.h"
#include "vector.h"
#include "input.h"
#include "hashtable.h"

bool ui_visible = false;
UIAction ui_current_action;
Array<UIElement>    ui_elements;
Array<UIPanel>      ui_panels;
Array<UIButton>     ui_buttons;
Array<UISlider>     ui_sliders;
Array<UITextField>  ui_text_fields;
UIElement* ui_hot       = nullptr;
UIPanel*   ui_hot_panel = nullptr;

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
    ui_current_action.element  = nullptr;
    ui_current_action.lock_hot = false;
}

void ui_reset() {
    array_reset(&ui_elements);
    array_reset(&ui_buttons);
    array_reset(&ui_panels);

    ui_init();
}

static UIElement* ui_create_element(Rect rect, UIElementType type, String name) {
    UIElement e;
    e.rect = rect;
    e.name = name;
    e.type = type;
    return array_add(&ui_elements, e);
}

UIPanel* ui_create_panel(Rect rect, String name) {
    UIElement* element = ui_create_element(rect, UIElementType::PANEL, name);
    
    UIPanel p{};
    p.element = element;
    auto panel = array_add(&ui_panels, p);;
    element->type_data = panel;

    return panel;
}

static void add_to_panel(UIPanel* panel, UIElement* element) {
    panel->children[panel->children_count++] = element;
}

UIButton* ui_create_button(Rect rect, String name) {
    UIElement* element = ui_create_element(rect, UIElementType::BUTTON, name);
    
    UIButton b{};
    b.element = element;
    auto button = array_add(&ui_buttons, b);
    element->type_data = button;

    return button;
}

UISlider* ui_create_slider(Rect rect, String name, float* value = nullptr) {
    UIElement* element = ui_create_element(rect, UIElementType::SLIDER, name);

    UISlider s{};
    s.value = value;
    s.target_value = *value;
    s.element = element;
    auto slider = array_add(&ui_sliders, s);
    element->type_data = slider;

    return slider;
}

UITextField* ui_create_text_field(Rect rect, String name) {
    UIElement* element = ui_create_element(rect, UIElementType::TEXT_FIELD, name);
    
    UITextField t{};
    t.element = element;
    auto field = array_add(&ui_text_fields, t);
    element->type_data = field;

    return field;
}

void ui_init() {
    ui_end_action();

    Rect rect = {0.2f, 0.2f, 0.3f, 0.6f};
    auto panel = ui_create_panel(rect, "MY_PANEL"_s);

    rect = pad_rect({0,0,1,1}, 0.1f);
    rect = cut_top(&rect, 0.2f);
    rect = pad_topbottom(rect, 0.75f);
    auto slider = ui_create_slider(rect, "SLIDER_R"_s, &entities[0].material.diffuse.r);
    slider->base_color = {1,0,0,1};
    add_to_panel(panel, slider->element);

    rect.y += rect.height;
    slider = ui_create_slider(rect, "SLIDER_G"_s, &entities[0].material.diffuse.g);
    slider->base_color = {0,1,0,1};
    add_to_panel(panel, slider->element);

    rect.y += rect.height;
    slider = ui_create_slider(rect, "SLIDER_B"_s, &entities[0].material.diffuse.b);
    slider->base_color = {0,0,1,1};
    add_to_panel(panel, slider->element);

    rect.y += rect.height;
    auto button = ui_create_button(rect, "BUTTON_RESUME"_s);
    button->base_color = {0,0,0,1};
    add_to_panel(panel, button->element);

    rect.y += rect.height;
    auto field = ui_create_text_field(rect, "FIELD_PEPEGA"_s);
    add_to_panel(panel, field->element);

    rect.y += rect.height * 2;
    field = ui_create_text_field(rect, "FIELD_PEPEGA2"_s);
    add_to_panel(panel, field->element);
    
    ui_visible = true;
}

void ui_update_hot() {
    ui_hot = nullptr;
    
    For(ui_panels) {
        Rect rect = it->element->rect;
        if(is_mouse_over(rect)) {
            ui_hot_panel = it;
            
            for(uint i = 0; i < it->children_count; i++) {
                UIElement* child = it->children[i];
                Rect child_rect = relative_to_screen(rect, child->rect);
                if(is_mouse_over(child_rect)) {
                    ui_hot = child;
                    return;
                }
            }

            ui_hot = it->element;
            return;
        }
    }
}

void ui_handle_click_event(EventKey* event) {
    auto active = ui_current_action.element;

    if(!ui_hot) {
        ui_end_action();
        return;
    }

    // We are making the assumption here, that a click outside the active widget ends the current action. This may or may not be true so we will have to see how this evolves.
    if(ui_hot != active) ui_end_action();

    // Using ui_hot here means that we don't allow real "in between frames" clicks, because we consider the click position to always be the the last position registered for the mouse in last frame, regardless of where the click really happened.
    // We do still register multiple clicks per frame if they happen, they just might not be exactly at the right position. I don't think we care about this right now, if at all, because the cases that make this implementation incorrect
    // would occur when clicking while moving the mouse REALLY fast, at which point you are just probably trolling with the UI instead of using it legitimately.
    // Doing it like this saves us from calculating the widget on which each click landed, we just assume all of them land on the hot element, which is highly likely.
    switch(ui_hot->type) {
        case UIElementType::BUTTON: {
            if(active == ui_hot) {
                if(event->keycode == VK_LBUTTON && !event->pressed) {
                    // In the future, when we use comple-time hashed strings the if-else chain below will become a switch statement.
                    if(equals(ui_hot->name, "BUTTON_RESUME"_s)) {
                        ui_visible = false;
                    }
                    else if(equals(ui_hot->name, "BUTTON_TEST"_s)) {
                        std::cout << "I clicked the other one" << std::endl;
                    }
                    
                    ui_end_action();
                }
            }
            else if(event->keycode == VK_LBUTTON && event->pressed) {
                ui_current_action.element = ui_hot;
            }
            break;
        }
    }
}

void ui_handle_text_event(EventText* event) {
    auto widget = ui_hot;

    switch(widget->type) {
        case UIElementType::TEXT_FIELD: {
            auto field = (UITextField*)widget->type_data;
            if(event->character == VK_BACK) {
                if(field->count > 0) field->count--;
            }
            else if(field->count < 64) field->value[field->count++] = event->character;
        }
    }
}

/*static void handle_widget_input(UIElement* element, bool is_active) {
    switch(element->type) {
        case UIElementType::BUTTON: {
            if(is_active) {
                if(input.left_click == KeyState::UP) {
                    if(element == ui_hot) {
                        // In the future, when we use comple-time hashed strings the if-else chain below will become a switch statement.
                        if(equals(element->name, "BUTTON_RESUME"_s)) {
                            ui_visible = false;
                        }
                        else if(equals(element->name, "BUTTON_TEST"_s)) {
                            std::cout << "I clicked the other one" << std::endl;
                        }
                    }
                    ui_end_action();
                }
            }
            else if(input.left_click == KeyState::DOWN) {
                ui_current_action.element = element;
            }
            break;
        }

        case UIElementType::PANEL: {
            if(is_active) {
                switch(ui_current_action.type) {
                    case UIActionType::MOVE: {
                        if(input.left_click == KeyState::UP) {
                            ui_end_action();
                        } else {
                            auto pivot = ui_current_action.move.pivot;
                            element->rect.x = input.mouse_pos_normalized.x - pivot.x;
                            element->rect.y = input.mouse_pos_normalized.y - pivot.y;
                        }
                        break;
                    }

                    case UIActionType::SIZE: {
                        if(input.right_click == KeyState::UP) {
                            ui_end_action();
                        }
                        else {
                            auto pivot = ui_current_action.size.pivot;
                            element->rect.width  = input.mouse_pos_normalized.x - element->rect.x + pivot.x;
                            element->rect.height = input.mouse_pos_normalized.y - element->rect.y + pivot.y;
                        }
                        break;
                    }
                }
            }
            else {
                Rect rect = ui_hot->rect;
                auto action = &ui_current_action;

                if(input.left_click == KeyState::DOWN) {
                    action->type = UIActionType::MOVE;
                    action->element = ui_hot;
                    action->lock_hot = true;
                    action->move.pivot.x = input.mouse_pos_normalized.x - rect.x;
                    action->move.pivot.y = input.mouse_pos_normalized.y - rect.y;
                }
                else if(input.right_click == KeyState::DOWN) {
                    action->type = UIActionType::SIZE;
                    action->element = ui_hot;
                    action->lock_hot = true;
                    action->size.pivot.x = rect.width  - (input.mouse_pos_normalized.x - rect.x);
                    action->size.pivot.y = rect.height - (input.mouse_pos_normalized.y - rect.y);
                }
            }
            break;
        }

        case UIElementType::SLIDER: {
            if(is_active) {
                if(input.left_click == KeyState::UP) {
                    ui_end_action();
                } else {
                    auto slider      = (UISlider*)element->type_data;
                    auto panel_rect  = ui_hot_panel->element->rect;
                    auto slider_rect = relative_to_screen(panel_rect, element->rect);
                    
                    auto slide_max       = slider_rect.width * 0.9f;
                    auto slide_distance  = input.mouse_pos_normalized.x - slider_rect.x - slider_rect.width * 0.05f;
                    slider->target_value = clamp(slide_distance / slide_max, 0.0f, 1.0f);
                }
            }
            else if(input.left_click == KeyState::DOWN) {
                ui_current_action.element  = element;
            }
            break;
        }

        case UIElementType::TEXT_FIELD: {
            if(is_active) {
                if(ui_hot != element && input.left_click == KeyState::DOWN) {
                    ui_end_action();
                }
                else {
                    auto field = (UITextField*)element->type_data;
                    For(events) {
                        if(it->type == EventType::TEXT) {
                            if(it->text.character == VK_BACK) {
                                if(field->count > 0) field->count--;
                            }
                            else if(field->count < 64) field->value[field->count++] = it->text.character;
                        }
                    }
                }
            }
            else if(input.left_click == KeyState::DOWN) {
                ui_current_action.element  = element;
            }
            break;
        }
    }
}

void ui_handle_input() {
    auto ui_active = ui_current_action.element;
    if(ui_visible) {
        if(!ui_current_action.lock_hot) {
            ui_update_hot();
            if(ui_hot) handle_widget_input(ui_hot, false);
        }
        
        if(ui_active) handle_widget_input(ui_active, true);
    }
    else if(ui_active) {
        ui_end_action();
    }
}*/

void ui_update() {
    For(ui_panels) {
        for(uint i = 0; i < it->children_count; i++) {
            UIElement* child = it->children[i];
            
            switch(child->type) {
                case UIElementType::SLIDER: {
                    auto slider = (UISlider*)child->type_data;
                    
                    if(fabs(slider->target_value - *slider->value) < 0.001f) {
                        *slider->value = slider->target_value;
                    }
                    else {
                        *slider->value += (slider->target_value - *slider->value) * 0.5f;
                    }
                    break;
                }
            }
        }    
    }
}

static void ui_flush(uint shader) {
    set_depth(false);
    set_blend(true);
    set_shader(shader);
    set_vertex_buffer(sd_vertex_buffer);
    modify_buffer(sd_vertex_buffer, sizeof(VertexPCU) * sd_vertices.count, sd_vertices.data);
    
    draw(sd_vertices.count);
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

static void draw_button(UIElement* element, UIPanel* panel) {
    auto button = (UIButton*)element->type_data;
    
    bool hovered = element == ui_hot;
    bool pressed = element == ui_current_action.element;
    
    Rect rect = relative_to_screen(panel->element->rect, element->rect);
    core_draw_button(rect, button->base_color, button->hover_color, hovered, pressed);
}

static void draw_slider(UIElement* element, UIPanel* panel) {
    auto slider = (UISlider*)element->type_data;

    auto slider_rect = relative_to_screen(panel->element->rect, element->rect);
    draw_rect(slider_rect, slider->base_color);

    Rect button = slider_rect;
    button.width *= 0.1f;
    button.x = slider_rect.x + (slider_rect.width - button.width) * *slider->value;
    bool pressed = ui_current_action.element == element;
    core_draw_button(button, slider->button_color, slider->button_color, true, pressed);
}

static void draw_text_field(UIElement* element, UIPanel* panel) {
    auto field = (UITextField*)element->type_data;

    auto field_rect = relative_to_screen(panel->element->rect, element->rect);
    
    auto selected_rect = pad_rect(field_rect, -0.2f);
    if(ui_current_action.element == element) draw_rect(selected_rect, {0,0,1,1});
    draw_rect(field_rect, {0.3f, 0.3f, 0.3f, 1.0f});

    String text = from_cstring(field->value, field->count);
    Vec2 origin = {field_rect.x, field_rect.y + field_rect.height};
    draw_text(text, origin, field_rect.height, {1,0,0,1});
}

static void draw_panel(UIPanel* panel) {
    draw_rect(panel->element->rect, panel->base_color);
    
    for(uint i = 0; i < panel->children_count; i++) {
        auto child = panel->children[i];
        switch(child->type) {
            case UIElementType::BUTTON: {
                draw_button(child, panel);
                break;
            }

            case UIElementType::SLIDER: {
                draw_slider(child, panel);
                break;
            }

            case UIElementType::TEXT_FIELD: {
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

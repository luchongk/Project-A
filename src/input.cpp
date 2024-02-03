#include "input.h"
#include "simulation.h"
#include "platform.h"
#include "entities.h"
//#include "ui.h"

PlayerInput input;

void init_input() {
    register_callbacks();
    load_keymaps();
}

void update_input(OSWindow* window) {
    Vec2i prev_mouse_pos_pixels = input.mouse_pos_pixels;
    input.mouse_raw_motion   = {0,0};
    input.mouse_delta_pixels = {0,0};
    
    os_poll_events(window);

    input.mouse_pos_normalized   = input.mouse_pos_pixels / window->size;
    input.mouse_delta_pixels     = input.mouse_pos_pixels - prev_mouse_pos_pixels;
    input.mouse_delta_normalized = input.mouse_delta_pixels / window->size;
    input.scroll = 0; //@Hack: Review which key/mouse events need resetting vs which don't.

    input.move.x = (float)(is_button_down(INPUT_BUTTON_RIGHT)  - is_button_down(INPUT_BUTTON_LEFT));
    input.move.y = (float)(is_key_down(VK_SPACE)      - is_key_down(VK_SHIFT));
    input.move.z = (float)(is_button_down(INPUT_BUTTON_BACK)   - is_button_down(INPUT_BUTTON_FORWARD));   // Flipped because -Z is forward.
    
    //ui_update_hot();
}

void set_mouse_pos(int x, int y) {
    input.mouse_pos_pixels = {x, y};
    SetCursorPos(x, y);
}

bool is_key_down(int vk_code) {
    return os_is_key_pressed(vk_code);
}

static void handle_key_event(EventKey* event) {
    u32  keycode = event->keycode;
    bool pressed = event->pressed;

    if(keycode == VK_LBUTTON || keycode == VK_RBUTTON) {
        //bool handled = ui_handle_click_event(event);
        //if(handled) return;
        
        if(!event->pressed) printf("Clicked the screen!\n");
    }

    //bool handled = ui_handle_key_event(event);
    //if(handled) return;

    bool is_repeat = event->is_repeat;
    
    if(!is_repeat) {
        switch(keycode) {
            case VK_LEFT: {
                if(pressed) my_time.sim_scale *= 0.5f;
                break;
            }

            case VK_RIGHT: {
                if(pressed) my_time.sim_scale *= 2.0f;
                break;
            }

            case VK_TAB: {
                if(pressed) {
                    //ui_visible = !ui_visible;
                    character_selected = !character_selected;
                }
                break;
            }

            case VK_F9: {
                if(pressed) {
                    bool is_fullscreen = window->fullscreen;
                    os_set_fullscreen(window, !is_fullscreen);
                }
                break;
            }

            case VK_F11: {
                if(pressed) {
                    bool is_fullscreen = window->fullscreen;
                    os_set_fullscreen(window, !is_fullscreen);
                }
                break;
            }
        }
    }
}

static void handle_text_event(EventText* event) {
    //if(ui_focused) ui_handle_text_event(event);
}

static void handle_window_event(EventWindow* event) {
    OSWindow* window = event->window;

    switch(event->type) {
        case EventWindowType::RESIZE: {
            if(!window->minimized) {
                int width  = (int)window->size.x;
                int height = (int)window->size.y;
                set_onscreen_framebuffer_size(width, height);
                
                if(using_perspective) {
                    set_perspective_projection((float)window->size.x, (float)window->size.y);
                }
                else {
                    set_orthographic_projection(20, 20.0f * window->size.y / window->size.x);
                }
            }
            //fallthrough
        }
        case EventWindowType::FOCUS_LOST: {
            input.move = {0,0};
            break;
        }
    }
}

bool handle_input(Array<Event>* events) {
    For(*events) {
        switch(it->type) {
            case EventType::QUIT: return true;
            
            case EventType::KEY: {
                auto button = keymap[it->key.keycode];
                if(button) {
                    auto callback = input_buttons[button].callback;
                    if(callback) callback(it->key.pressed, it->key.is_repeat);
                }
                break;
            }

            case EventType::TEXT: {
                handle_text_event(&it->text);
                break;
            }

            case EventType::SCROLL: {
                input.scroll = it->scroll.amount;
                break;
            }

            case EventType::WINDOW: {
                handle_window_event(&it->window);
                break;
            }
        }
    }

    return false;
}
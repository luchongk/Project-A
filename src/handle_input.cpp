#include "handle_input.h"
#include "simulation.h"
#include "entities.h"
#include "input.h"
#include "general.h"
#include "windowing.h"

static void handle_key_event(OsEvent* event) {
    auto key = event->key;
    /*if(keycode == VK_LBUTTON || keycode == VK_RBUTTON) {
        //bool handled = ui_handle_click_event(event);
        //if(handled) return;
        
        if(!event->pressed) printf("Clicked the screen!\n");
    }*/

    //bool handled = ui_handle_key_event(event);
    //if(handled) return;
    
    switch(key.keycode) {
        case VK_LEFT: {
            if(key.state & JUST_PRESSED) my_time.sim_scale *= 0.5f;
            break;
        }

        case VK_RIGHT: {
            if(key.state & JUST_PRESSED) my_time.sim_scale *= 2.0f;
            break;
        }

        case VK_TAB: {
            if(key.state & JUST_PRESSED) {
                //ui_visible = !ui_visible;
                character_selected = !character_selected;
            }
            break;
        }

        case VK_F11: {
            if(key.state & JUST_PRESSED) {
                bool fullscreen = is_fullscreen(event->window);
                set_fullscreen(event->window, !fullscreen);
            }
            break;
        }
    }
}

static void handle_text_event(OsEvent* event) {
    //if(ui_focused) ui_handle_text_event(event);
}

bool handle_input(Array<OsEvent>* events) {

    For(*events) {
        switch(it.type) {
            case OsEventType::WINDOW_CLOSED: return true;

            case OsEventType::WINDOW_RESIZED: {
                auto window = it.window;
                if(!is_minimized(window)) {
                    auto resize = it.resize;
                    set_onscreen_framebuffer_size(resize.width, resize.height);
                    
                    if(using_perspective) {
                        set_perspective_projection((float)resize.width, (float)resize.height);
                    }
                    else {
                        set_orthographic_projection(20, 20.0f * resize.height / resize.width);
                    }
                }
                //fallthrough
            }

            case OsEventType::WINDOW_FOCUS_LOST: {
                main_player->move = {0,0};
                break;
            }
            
            case OsEventType::KEY: {
                /*auto button = keymap[it->key.keycode];
                if(button) {
                    auto callback = input_buttons[button].callback;
                    if(callback) callback(it->key.pressed, it->key.is_repeat);
                }*/
                handle_key_event(&it);
                break;
            }

            case OsEventType::TEXT: {
                handle_text_event(&it);
                break;
            }

            
        }
    }

    main_player->move.x = (float)((bool)(keystates['D'] & IS_DOWN) - (bool)(keystates['A'] & IS_DOWN));
    main_player->move.y = (float)((bool)(keystates[VK_SPACE] & IS_DOWN) - (bool)(keystates[VK_SPACE] & IS_DOWN));
    main_player->move.z = (float)((bool)(keystates['S'] & IS_DOWN) - (bool)(keystates['W'] & IS_DOWN));   // Flipped because -Z is forward.

    return false;
}
#include "handle_input.h"
#include "simulation.h"
#include "entities.h"
#include "input.h"
#include "general.h"
#include "windowing.h"
#include "ui.h"

static void handle_key_event(OsEvent* event) {
    auto key = event->key;
    
    switch(key.keycode) {
        case VK_LEFT: {
            if(key.state & JUST_PRESSED) my_time.sim_scale *= 0.5f;
            break;
        }

        case VK_RIGHT: {
            if(key.state & JUST_PRESSED) my_time.sim_scale *= 2.0f;
            break;
        }

        case 'R': {
            if(key.state & JUST_PRESSED) reset_scene();
            break;
        }

        case VK_TAB: {
            if(key.state & JUST_PRESSED) {
                devtools_open = !devtools_open;
                //character_selected = !character_selected;
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

bool handle_input() {

    ui_resolve_input(&os_events, mouse_position.x, mouse_position.y, my_time.dt);

    for(int i = 0; i < os_events.count; i++) {
        auto& e = os_events[i];

        switch(e.type) {
            case OsEventType::WINDOW_CLOSED: return true;

            case OsEventType::WINDOW_RESIZED: {
                auto window = e.window;
                if(!is_minimized(window)) {
                    renderer_on_resize(e.resize.width, e.resize.height);
                }
                //fallthrough
            }
            
            case OsEventType::KEY: {
                /*auto button = keymap[it->key.keycode];
                if(button) {
                    auto callback = input_buttons[button].callback;
                    if(callback) callback(it->key.pressed, it->key.is_repeat);
                }*/
                handle_key_event(&e);
                break;
            }

            case OsEventType::TEXT: {
                handle_text_event(&e);
                break;
            }

            
        }
    }

    main_player->move.x = (float)((bool)(keystates['D'] & IS_DOWN) - (bool)(keystates['A'] & IS_DOWN));
    main_player->move.y = (float)(bool)(keystates[VK_SPACE] & IS_DOWN);
    main_player->move.z = (float)((bool)(keystates['S'] & IS_DOWN) - (bool)(keystates['W'] & IS_DOWN));   // Flipped because -Z is forward.

    ui_input_pass();

    return false;
}
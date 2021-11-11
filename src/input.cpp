#include "input.h"
#include "simulation.h"
#include "platform.h"
#include "entities.h"
#include "ui.h"

PlayerInput input;

void update_input() {
    os_poll_events();

    input.mouse_pos_normalized   = input.mouse_pos_pixels   / window->size;
    input.mouse_delta_normalized = input.mouse_delta_pixels / window->size;
}

static void handle_key_event(EventKey* event) {
    u32  keycode   = event->keycode;
    bool pressed   = event->pressed;
    bool is_repeat = event->is_repeat;

    if(!is_repeat) {
        switch(keycode) {
            case 'W': {
                input.move.z += pressed ? 1 : -1;
                break;
            }

            case 'S': {
                input.move.z -= pressed ? 1 : -1;
                break;
            }
            
            case 'A': {
                input.move.x -= pressed ? 1 : -1;
                break;
            }

            case 'D': {
                input.move.x += pressed ? 1 : -1;
                break;
            }

            case VK_SPACE: {
                input.move.y += pressed ? 1 : -1;
                break;
            }

            case VK_SHIFT: {
                input.move.y -= pressed ? 1 : -1;
                break;
            }

            case 'Q': {
                input.rotation -= pressed ? 1 : -1;
                break;
            }

            case 'E': {
                input.rotation += pressed ? 1 : -1;
                break;
            }

            case 'P': {
                if(pressed) {
                    paused = !paused;
                    //os_show_mouse(paused);
                }
                break;
            }

            case 'R': {
                if(pressed && !ui_current_action.element) {
                    reset_scene();
                    cubes_rotation = 0;
                    time.start_stamp = os_get_timestamp();
                    time.since_start = 0;
                    ui_reset();
                }
                break;
            }

            case VK_LEFT: {
                if(pressed) time.sim_scale *= 0.5f;
                break;
            }

            case VK_RIGHT: {
                if(pressed) time.sim_scale *= 2.0f;
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
                    os_set_fullscreen(window, !is_fullscreen, false);
                }
                break;
            }

            case VK_F11: {
                if(pressed) {
                    bool is_fullscreen = window->fullscreen;
                    os_set_fullscreen(window, !is_fullscreen, true);
                }
                break;
            }

            case VK_LBUTTON:
            case VK_RBUTTON: {
                ui_handle_click_event(event);
                break;
            }
        }
    }
}

static void handle_text_event(EventText* event) {
    if(ui_hot) {
        ui_handle_text_event(event);
    }
}

static void handle_window_event(EventWindow* event) {
    OSWindow* window = event->window;

    switch(event->type) {
        case EventWindowType::RESIZE: {
            int width  = (int)window->size.x;
            int height = (int)window->size.y;
            set_projection(width, height);
            set_framebuffer_size(width, height);
            
            break;
        }

        case EventWindowType::FOCUS_LOST: {
            input.move = {0,0};
            input.rotation = 0;

            if(window->fullscreen && !window->borderless) {
                set_fullscreen(false);
                os_minimize_window(window);
            }
            break;
        }
        
        case EventWindowType::FOCUS_GAINED: {
            if(window->fullscreen) {
                if(!window->borderless) {
                    os_restore_window(window);
                    set_fullscreen(true);
                }
            }
            break;
        }
    }
}

bool handle_input(Array<Event>* events) {
    /*if(window->fullscreen && window->focused) {
        os_set_mouse_to_center(window);
    }*/

    ui_update_hot();

    For(*events) {
        switch(it->type) {
            case EventType::QUIT: return true;
            
            case EventType::KEY: {
                //TODO: Here we would call the keymapper instead of passing the keycode directly to the function below
                handle_key_event(&it->key);
                break;
            }

            case EventType::TEXT: {
                handle_text_event(&it->text);
            }

            case EventType::WINDOW: {
                handle_window_event(&it->window);
                break;
            }
        }
    }

    return false;
}
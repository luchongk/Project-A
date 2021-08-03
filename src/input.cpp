#include "input.h"
#include "simulation.h"
#include "platform.h"
#include "entities.h"

PlayerInput player_input;

void update_input() {
    os_poll_events();

    Vec2 actual_size = window->fullscreen ? Vec2{1920, 1080} : window->size;
    player_input.mouse_pos_normalized.x = player_input.mouse_pos_screen.x / actual_size.x;
    player_input.mouse_pos_normalized.y = player_input.mouse_pos_screen.y / actual_size.y;

    player_input.left_click = KeyState::NONE;
}

static void handle_key_event(u32 keycode, bool pressed, bool is_repeat) {
    if(!is_repeat) {
        switch(keycode) {
            case 'W': {
                player_input.move.z += pressed ? 1 : -1;
            } break;

            case 'S': {
                player_input.move.z -= pressed ? 1 : -1;
            } break;
            
            case 'A': {
                player_input.move.x -= pressed ? 1 : -1;
            } break;

            case 'D': {
                player_input.move.x += pressed ? 1 : -1;
            } break;

            case VK_SPACE: {
                player_input.move.y += pressed ? 1 : -1;
            } break;

            case VK_SHIFT: {
                player_input.move.y -= pressed ? 1 : -1;
            } break;

            case 'Q': {
                player_input.rotation -= pressed ? 1 : -1;
            } break;

            case 'E': {
                player_input.rotation += pressed ? 1 : -1;
            } break;

            case 'P': {
                if(pressed) {
                    paused = !paused;
                    os_show_mouse(paused);
                }
            } break;

            case 'R': {
                if(pressed) {
                    reset_scene();
                    cubes_rotation = 0;
                }
            } break;

            case VK_LEFT: {
                if(pressed) time.sim_scale *= 0.5f;
            } break;

            case VK_RIGHT: {
                if(pressed) time.sim_scale *= 2.0f;
            } break;

            case VK_TAB: {
                if(pressed) {
                    character_selected = !character_selected;
                }
            } break;

            case VK_F9: {
                if(pressed) {
                    bool is_fullscreen = window->fullscreen;
                    os_show_mouse(paused);
                    os_set_fullscreen(window, !is_fullscreen, false);
                }
            } break;

            case VK_F11: {
                if(pressed) {
                    bool is_fullscreen = window->fullscreen;
                    os_show_mouse(paused);
                    os_set_fullscreen(window, !is_fullscreen, true);
                }
            } break;

            case VK_LBUTTON: {
                if(pressed) {
                    player_input.left_click = KeyState::DOWN;
                }
                else {
                    player_input.left_click = KeyState::UP;
                }
            } break;
        }
    }
}

void handle_window_event(EventWindowType type, OSWindow* window, int data1, int data2) {
    switch(type) {
        case EventWindowType::RESIZE: {
            adjust_projection(data1, data2);
            adjust_size(data1, data2);
            
            if(!window->fullscreen) window->size = {(float)data1, (float)data2};
        } break;

        case EventWindowType::FOCUS_LOST: {
            player_input.move = {0,0};
            player_input.rotation = 0;

            if(window->fullscreen && !window->borderless) {
                set_fullscreen(false);
                os_minimize_window(window);
            }
        } break;
        
        case EventWindowType::FOCUS_GAINED: {
            if(window->fullscreen) {
                if(!window->borderless) {
                    os_restore_window(window);
                    set_fullscreen(true);
                }
            }
        } break;
    }
}

bool handle_input(Array<Event>* events) {
    /*if(window->fullscreen && window->focused) {
        os_set_mouse_center(window);
    }*/

    For(*events) {
        switch(it->type) {
            case EventType::QUIT: return true;
            
            case EventType::KEY: {
                //TODO: Here we would call the keymapper instead of passing the keycode directly to the function below
                handle_key_event(it->key.keycode, it->key.pressed, it->key.is_repeat);
            } break;

            case EventType::WINDOW: {
                handle_window_event(it->window.type, it->window.window, it->window.data1, it->window.data2);
            } break;
        }
    }

    return false;
}
#include "input.h"
#include "simulation.h"
#include "platform.h"
#include "entities.h"

PlayerControls controls;

static void handle_key_event(u32 keycode, bool pressed, bool is_repeat) {
    if(!is_repeat) {
        switch(keycode) {
            case 'W': {
                controls.move.z += pressed ? 1 : -1;
                break;
            }

            case 'S': {
                controls.move.z -= pressed ? 1 : -1;
                break;
            }
            
            case 'A': {
                controls.move.x -= pressed ? 1 : -1;
                break;
            }

            case 'D': {
                controls.move.x += pressed ? 1 : -1;
                break;
            }

            case VK_SPACE: {
                controls.move.y += pressed ? 1 : -1;
                break;
            }

            case VK_SHIFT: {
                controls.move.y -= pressed ? 1 : -1;
                break;
            }

            case 'Q': {
                controls.rotation -= pressed ? 1 : -1;
                break;
            }

            case 'E': {
                controls.rotation += pressed ? 1 : -1;
                break;
            }

            case 'P': {
                if(pressed) {
                    paused = !paused;
                    os_lock_mouse(paused ? nullptr : window);
                }
                break;
            }

            case 'R': {
                if(pressed) {
                    reset_entities();
                    cubes_rotation = 0;
                }
                break;
            }

            case VK_LEFT: {
                if(pressed) {
                    time.sim_scale *= 0.5f;
                }
                break;
            }

            case VK_RIGHT: {
                if(pressed) {
                    time.sim_scale *= 2.0f;
                }
                break;
            }

            case VK_TAB: {
                if(pressed) {
                    character = !character;
                }
                break;
            }

            case VK_F9: {
                if(pressed) {
                    bool is_fullscreen = os_is_fullscreen(window);
                    os_set_fullscreen(window, !is_fullscreen, false);
                }
                break;
            }

            case VK_F11: {
                if(pressed) {
                    bool is_fullscreen = os_is_fullscreen(window);
                    os_set_fullscreen(window, !is_fullscreen, true);
                }
                break;
            }
        }
    }
}

void handle_event(Event* e) {
    switch(e->type) {
        case EventType::KEY: {
            //TODO: Here we would call the keymapper instead of passing the keycode directly to the function below
            handle_key_event(e->key.keycode, e->key.pressed, e->key.is_repeat);
            break;
        }
    }
}
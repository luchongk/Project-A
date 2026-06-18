#include "handle_input.h"
#include "simulation.h"
#include "physics.h"
#include "entities.h"
#include "input.h"
#include "general.h"
#include "windowing.h"
#include "ui.h"
#include "editor.h"

static void handle_key_event(OsEvent* event) {
    auto key = event->key;
    
    switch(key.keycode) {
        case VK_LBUTTON: {
            if(key.state & JUST_PRESSED) {                
                auto c = get_entity(main_camera);
                if(c) {
                    Entity* closest = nullptr;
                    float closest_t = MAX_FLOAT;
                    ForP(manager->entities.all) {
                        if((it->flags & (ENTITY_FLAG_ACTIVE | ENTITY_FLAG_MOUSE_PICKABLE)) != (ENTITY_FLAG_ACTIVE | ENTITY_FLAG_MOUSE_PICKABLE)) continue;

                        auto t = raycast(c->position, c->camera->forward, get_world_space_collider(it));
                        if(t > 0.0001f && t < closest_t) {
                            closest = it;
                            closest_t = t;
                        }
                    }
                    if(closest) {
                        editor_selected = closest->handle;
                    } else {
                        editor_selected = INVALID_ENTITY_HANDLE;
                    }
                }
            }
            break;
        }

        case VK_LEFT: {
            if(key.state & JUST_PRESSED) my_time.sim_scale *= 0.5f;
            break;
        }

        case VK_RIGHT: {
            if(paused && key.state & IS_DOWN) {
                do_step = true;
            }
            else if(key.state & JUST_PRESSED) {
                my_time.sim_scale *= 2.0f;
            }
            break;
        }

        case 'P': {
            if(key.state & JUST_PRESSED) paused = !paused;
            break;
        }

        case 'R': {
            if(key.state & JUST_PRESSED) reset_scene();
            break;
        }

        case 'T': {
            if(key.state & JUST_PRESSED) using_perspective = !using_perspective;
            break;
        }

        case VK_TAB: {
            if(key.state & JUST_PRESSED) {
                editor_on = !editor_on;
                capture_and_hide_mouse(event->window, !editor_on);
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

    ui_resolve_interactables(&os_events, mouse_position.x, mouse_position.y, my_time.dt);

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

    return false;
}
#include "input.h"
#include "simulation.h"
#include "platform.h"

void handle_events(OSWindow* window, PlayerInput* input) {
    if(!window->focused) return;

    input->mouse_delta = os_events.mouse_delta;

    For(os_events.keyboard) {
        if(it->pressed) {
            switch(it->code) {
                case 'P': {
                    paused = !paused;
                    continue;
                }

                case 'R': {
                    camera = {};
                    cubes_rotation = 0;
                    light_time_accum = 0;
                    continue;
                }

                case VK_LEFT: {
                    time.modifier *= 0.5f;
                    continue;
                }

                case VK_RIGHT: {
                    time.modifier *= 2.0f;
                    continue;
                }

                case VK_F11: {
                    os_toggle_fullscreen(window, true);
                    continue;
                }
            }
        }
        
        switch(it->code) {
            case 'A': {
                input->horizontal += it->pressed ? -1 : 1;
                break;
            }

            case 'D': {
                input->horizontal += it->pressed ? 1 : -1;
                break;
            }

            case 'S': {
                input->vertical += it->pressed ? -1 : 1;
                break;
            }

            case 'W': {
                input->vertical += it->pressed ? 1 : -1;
                break;
            }
        }
    }
}
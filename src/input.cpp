#include "input.h"
#include "simulation.h"
#include "platform.h"

void handle_events(OSWindow* window, PlayerInput* input) {
    For(os_events.keyboard) {
        if(it->pressed) {
            switch(it->code) {
                case 'P': {
                    paused = !paused;
                    os_lock_cursor(paused ? nullptr : window);
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

                case VK_F9: {
                    bool is_fullscreen = os_is_fullscreen(window);
                    os_set_fullscreen(window, !is_fullscreen, false);
                    continue;
                }

                case VK_F11: {
                    bool is_fullscreen = os_is_fullscreen(window);
                    os_set_fullscreen(window, !is_fullscreen, true);
                    continue;
                }
            }
        }
    }

    input->mouse_delta = os_events.mouse_delta;

    input->horizontal = -(int)os_keyboard['A'] + (int)os_keyboard['D'];
    input->vertical   = -(int)os_keyboard['S'] + (int)os_keyboard['W'];
}
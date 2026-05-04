#include "strings.h"
#include "hashtable.h"
#include "platform.h"
#include "simulation.h"
#include "entities.h"
#include "main.h"
#include "enum_with_names.h"

typedef void (*InputCallback)(bool pressed, bool is_repeat);

struct MappedButton {
    InputCallback callback;
    u8 keys[2];
};

MappedButton input_buttons[256]; // Indexed by InputButton enum.
int keymap[256]; // Keycode to InputButton.

#define ENUM_INPUT_BUTTON(X) \
    X(INPUT_BUTTON_NONE,               ""_s) \
    X(INPUT_BUTTON_FORWARD,            "forward"_s) \
    X(INPUT_BUTTON_BACK,               "back"_s) \
    X(INPUT_BUTTON_LEFT,               "left"_s) \
    X(INPUT_BUTTON_RIGHT,              "right"_s) \
    X(INPUT_BUTTON_ROTATE_LEFT,        "rotate_left"_s) \
    X(INPUT_BUTTON_ROTATE_RIGHT,       "rotate_right"_s) \
                                                         \
    X(INPUT_BUTTON_PAUSE,              "pause"_s) \
    X(INPUT_BUTTON_RESET_SCENE,        "reset_scene"_s) \
    X(INPUT_BUTTON_SWITCH_PERSPECTIVE, "switch_perspective"_s) \
    X(INPUT_BUTTON_STEP_SIMULATION,    "step_simulation"_s) \
    X(INPUT_BUTTON_WEIRD,              "weird"_s) \
    X(INPUT_BUTTON_DO_THE_THING,       "do_the_thing"_s) \

DECLARE_ENUM(InputButton, ENUM_INPUT_BUTTON)
DEFINE_ENUM(InputButton,  ENUM_INPUT_BUTTON)

void load_keymaps() {
    Array<u8> data;
    os_read_entire_file("assets/inputs.keymap"_s, &data);

    String iter = (String)data;
    while(iter.count > 0) {
        auto line = eat_line(&iter);
        if(line.count == 0) continue;

        auto name = eat_until(' ', &line);
        
        line = advance(line, 1);
        auto key = eat_until('\n', &line);

        auto button = InputButton_get_value(name);
        if(button) {
            input_buttons[button].keys[0] = key[0];
            keymap[key[0]] = button;
        }
    }

    free_(data.data);
}

bool is_button_down(InputButton b) {
    auto button = input_buttons[b];
    return os_is_key_pressed(button.keys[0]) || os_is_key_pressed(button.keys[1]);
}

// User defined callbacks below:

void pause_callback(bool pressed, bool is_repeat) {
    if(pressed && !is_repeat) paused = !paused;
}

void reset_scene_callback(bool pressed, bool is_repeat) {
    if(pressed && !is_repeat) {
        reset_scene();
        my_time.start_stamp = os_get_timestamp();
        my_time.since_start = 0;
    }
}

void switch_perspective_callback(bool pressed, bool is_repeat) {
    if(pressed && !is_repeat) {
        if(using_perspective) {
            using_perspective = false;
            set_orthographic_projection(ORTHOGRAPHIC_VIEW_WIDTH, ORTHOGRAPHIC_VIEW_WIDTH * window->size.y / window->size.x);
        }
        else {
            using_perspective = true;
            set_perspective_projection((float)window->size.x, (float)window->size.y);
        }
    }
}

void step_simulation_callback(bool pressed, bool is_repeat) {
    if(pressed) do_step = true;
}

void register_callbacks() {
    input_buttons[INPUT_BUTTON_PAUSE].callback              = pause_callback;
    input_buttons[INPUT_BUTTON_RESET_SCENE].callback        = reset_scene_callback;
    input_buttons[INPUT_BUTTON_SWITCH_PERSPECTIVE].callback = switch_perspective_callback;
    input_buttons[INPUT_BUTTON_STEP_SIMULATION].callback    = step_simulation_callback;
}
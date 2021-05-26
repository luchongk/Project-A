#include "input.h"
#include "simulation.h"
#include "platform.h"
#include "entities.h"

Input input;

void input_next_frame() {
    input.mouse_delta = {0,0};
    for(int i = 0; i < 256; i++) {
        input.keys[i].last = input.keys[i].current;
    }
    input.text.count = 0;
    //os_events.other.count = 0;
}

void clear_keys() {
    for(int i = 0; i < 256; i++) {
        input.keys[i].current = false;
    }
}

void clear_input() {
    input.mouse_delta = {0,0};
    clear_keys();
    input.text.count = 0;
}

bool key_down(u32 code) {
    KeyState key = input.keys[code];

    return key.current && !key.last;
}

bool key_up(u32 code) {
    KeyState key = input.keys[code];
    
    return key.last && !key.current;
}

bool key_pressed(u32 code) {
    KeyState key = input.keys[code];
    
    return key.current;
}

int key_axis(u32 negative, u32 positive) {
    return -(int)input.keys[negative].current + (int)input.keys[positive].current;
}
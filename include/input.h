#ifndef INPUT_H
#define INPUT_H

struct KeyState {
    bool last;
    bool current;
};

struct Input {
    Vec2 mouse_delta;
    KeyState keys[256];
    
    //@Temporary
    Array<char> text;
};

extern Input input;

void input_next_frame();
void clear_keys();
void clear_input();

bool key_down(u32 code);
bool key_up(u32 code);
bool key_pressed(u32 code);
int key_axis(u32 negative, u32 positive);

#endif
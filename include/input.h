#ifndef INPUT_H
#define INPUT_H

enum class EventType {
    QUIT,
    KEY,
    WINDOW,
};

enum class EventWindowType {
    FOCUS_GAINED,
    FOCUS_LOST,
    RESIZE
};

struct EventKey {
    u32 keycode;
    bool pressed;
    bool is_repeat;
};

struct EventWindow {
    EventWindowType type;
    OSWindow* window;
    int data1;
    int data2;
};

struct Event {
    EventType type;
    union {
        EventKey key;
        EventWindow window;
    };
};


enum class KeyState : u8 {
    NONE,
    DOWN,
    UP
};

struct PlayerInput {
    KeyState left_click;
    bool mouse_interacting_with_ui = false;
    Vec2 mouse_pos_screen;
    Vec2 mouse_pos_normalized;
    Vec2 mouse_delta;
    Vec3 move;
    float rotation;
};

extern PlayerInput player_input;

void update_input();
bool handle_input(Array<Event>* events);

#endif
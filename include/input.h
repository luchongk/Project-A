#ifndef INPUT_H
#define INPUT_H

enum class EventType {
    QUIT,
    KEY,
    TEXT,
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

struct EventText {
    char character;
};

struct EventWindow {
    EventWindowType type;
    OSWindow* window;
};

struct Event {
    EventType type;
    union {
        EventKey key;
        EventText text;
        EventWindow window;
    };
};

struct PlayerInput {
    Vec2 mouse_pos_pixels;
    Vec2 mouse_pos_normalized;
    Vec2 mouse_delta_pixels;
    Vec2 mouse_delta_normalized;
    Vec3 move;
    float rotation;
    bool jump;
};

extern PlayerInput input;

void update_input();
bool handle_input(Array<Event>* events);

#endif
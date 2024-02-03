#ifndef INPUT_H
#define INPUT_H

enum class EventType {
    QUIT,
    KEY,
    TEXT,
    SCROLL,
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

struct EventScroll {
    int amount;
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
        EventScroll scroll;
        EventWindow window;
    };
};

struct PlayerInput {
    Vec2i mouse_raw_motion;
    Vec2i mouse_pos_pixels;
    Vec2 mouse_pos_normalized;
    Vec2i mouse_delta_pixels;
    Vec2 mouse_delta_normalized;
    Vec3 move;
    int scroll;
};

extern PlayerInput input;

void update_input();
bool handle_input(Array<Event>* events);
void set_mouse_pos(int x, int y);
bool is_key_down(int vk_code);

#endif
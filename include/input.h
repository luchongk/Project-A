#ifndef INPUT_H
#define INPUT_H

enum class EventType {
    KEY
};

struct EventKey {
    u32 keycode;
    bool pressed;
    bool is_repeat;
};

struct Event {
    EventType type;
    union {
        EventKey key;
    };
};

struct PlayerControls {
    Vec2 mouse_delta;
    Vec3 move;
    float rotation;
};

extern PlayerControls controls;

void handle_event(Event* e);

#endif
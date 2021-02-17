#ifndef INPUT_H
#define INPUT_H

struct PlayerInput {
    int horizontal;
    int vertical;
    Vector2 mouse_delta;
};

void handle_events(OSWindow* window, PlayerInput* input);

#endif
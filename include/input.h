#ifndef INPUT_H
#define INPUT_H

struct PlayerInput {
    int horizontalX;
    int horizontalZ;
    int vertical;
    int rotationDir;
    Vector2 mouse_delta;
};

void handle_events(OSWindow* window, PlayerInput* input);

#endif
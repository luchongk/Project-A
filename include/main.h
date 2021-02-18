#ifndef GAME_H
#define GAME_H

struct Time {
    s64 start_stamp;
    s64 last_stamp;
    s64 frequency;
    float since_start;
    float delta;
    float simulation_delta;
    float modifier;
};

extern Time time;
extern float maxFrameTime;

#endif
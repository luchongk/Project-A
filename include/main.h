#ifndef GAME_H
#define GAME_H

struct Time {
    u64 start_stamp;
    u64 last_frame_stamp;
    float since_start;
    float max_allowed_per_frame;
    float dt;
    float sim_dt;
    float sim_scale;
};

extern Time time;
extern OSWindow* window;
extern bool mouse_locked;

#endif
#ifndef GAME_H
#define GAME_H

struct Time {
    u64 start_stamp;
    u64 last_frame_stamp;
    float since_start;
    float max_dt_allowed;
    float dt;
    float sim_dt;
    float sim_scale;
};

extern Time my_time;
extern OSWindow* window;
extern LinearArena temporary_storage;

#endif
#ifndef GAME_H
#define GAME_H

#include "types.h"
#include "memory.h"

// TODO: It would be nice to have a "since_start" that doesn't add time while we are paused.
struct Time {
    u64 start_stamp;
    u64 last_frame_stamp;
    float since_start;
    float max_dt_allowed;
    float dt;
    float simulation_dt;
    float sim_scale;
};

extern Time my_time;
extern OSWindow* window;
extern LinearArena temporary_storage;

#endif
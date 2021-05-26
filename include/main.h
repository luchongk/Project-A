#ifndef GAME_H
#define GAME_H

struct Time {
    s64 start_stamp;
    s64 last_stamp;
    s64 frequency;
    float since_start;
    float dt;
    float sim_dt;
    float scaled_sim_dt;
    float modifier;
};

extern Time time;
extern float max_frame_time;

#endif
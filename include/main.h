#ifndef GAME_H
#define GAME_H

struct Time {
    float start_at;
    float since_start;
    float max_allowed_per_frame;
    float last_frame;
    float dt;
    float sim_dt;
    float sim_scale;
};

extern Time time;
extern OSWindow* window;

#endif
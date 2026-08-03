#ifndef GENERAL_H
#define GENERAL_H

#include "base/types.h"

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
extern OsWindow the_window;
extern const float ORTHOGRAPHIC_VIEW_WIDTH;
extern bool editor_on;
extern bool do_step;

#endif
#ifndef SIMULATION_H
#define SIMULATION_H

#include "vector.h"

struct Camera {
    Vector3 position;
    Vector3 forward = {0, 0, -1};
    float yaw = -90;
    float pitch;
};

//All these are @TEMPORARY
extern Camera camera;
extern Vector3 light_pos;
extern float cubes_rotation;
extern bool paused;
extern float light_time_accum;

struct PlayerInput;
void simulate(PlayerInput* input);

#endif
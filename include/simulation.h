#ifndef SIMULATION_H
#define SIMULATION_H

#include "vector.h"

//All these are @TEMPORARY
extern Vec3 light_pos;
extern float cubes_rotation;
extern Vec3 cubes_offset;
extern bool paused;
extern bool character;
extern float light_time_accum;

struct PlayerInput;
void simulate(PlayerInput* input);

#endif
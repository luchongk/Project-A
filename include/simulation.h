#ifndef SIMULATION_H
#define SIMULATION_H

#include "vector.h"

//All these are @TEMPORARY
extern float cubes_rotation;
extern bool paused;
extern bool character;

struct PlayerInput;
void simulate(PlayerInput* input);

#endif
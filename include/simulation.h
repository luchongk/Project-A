#ifndef SIMULATION_H
#define SIMULATION_H

#include "maths.h"

//All these are @TEMPORARY
extern bool paused;
extern Vector2i saved_mouse_pos;
extern char velocity_strings[64][128];

void update_camera(float dt);
void simulate(float dt);

#endif
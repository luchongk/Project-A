#ifndef SIMULATION_H
#define SIMULATION_H

#include "maths.h"

//All these are @TEMPORARY
extern float cubes_rotation;
extern bool paused;
extern bool character_selected;
extern float do_the_thing_cooldown;
extern Vector2i saved_mouse_pos;
char velocity_strings[64][128];

void update_camera(float dt);
void simulate(float dt);

#endif
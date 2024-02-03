#ifndef SIMULATION_H
#define SIMULATION_H

//All these are @TEMPORARY
extern float cubes_rotation;
extern bool paused;
extern bool character_selected;
extern Vec2i saved_mouse_pos;
char velocity_strings[64][128];

void simulate(float dt);

#endif
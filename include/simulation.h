#ifndef SIMULATION_H
#define SIMULATION_H

#include "maths.h"

struct OsEvent;

extern bool paused;

void process_game_input(OsEvent* event);
void update_camera(float dt);
void simulate(float dt);

#endif
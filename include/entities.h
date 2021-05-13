#ifndef ENTITY_H
#define ENTITY_H

#include "obj_loader.h"
#include "matrix.h"

struct Camera {
    Vec3 position = {0,0,2};
    float yaw;
    float pitch;
    Vec3 forward;
};

struct Entity {
    Vec3 position;
    Vec3 scale;
    Matrix orientation;
    Mesh* mesh;
};

extern Camera camera;
extern Entity entities[10];

#endif
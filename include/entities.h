#ifndef ENTITY_H
#define ENTITY_H

#include "obj_loader.h"
#include "matrix.h"
#include "render.h"

struct Camera {
    Vec3 position;
    float yaw;
    float pitch;
    Vec3 forward;
};

struct Entity {
    Vec3 position;
    float scale;
    Matrix orientation;
    Mesh* mesh;
    Material material;
};

struct Light : Entity {
    Vec3 ambient;
    Vec3 diffuse;
    Vec3 specular;
    float t_movement;
};

extern Camera camera;
extern Light light;
extern Entity entities[10];
extern int entities_count;

void reset_scene();

Matrix get_world_matrix(Entity* entity);

#endif
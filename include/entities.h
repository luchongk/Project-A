#ifndef ENTITY_H
#define ENTITY_H

#include "obj_loader.h"
#include "matrix.h"
#include "render.h"

ENUM(EntityType,
    CAMERA,
    LIGHT,
    GROUND,
)

const int MAX_ENTITIES = 100;

template<typename T>
struct EntityPool {
    static EntityType type;
    inline static T pool[MAX_ENTITIES];
    inline static int count;
};

struct Entity {
    EntityType type;
    Vec3 position;
    float scale;
    Matrix orientation;
    Mesh* mesh;
    Material* material;
    void* specific_data;
};

struct Light {
    Entity* entity;
    Vec3 ambient;
    Vec3 diffuse;
    Vec3 specular;
};

struct Camera {
    Entity* entity;
    float yaw;
    float pitch;
};

struct Ground {
    Entity* entity; //@Cleanup: This will probably be an ID instead of a pointer later.
    Vec3 dimensions;
};

extern const int MAX_ENTITIES;
extern Entity entities[];
extern int entity_count;

extern Ground pool_Ground[];
extern int count_Ground;

extern Camera* main_camera;
extern Light* light;

extern Ground* player;

void reset_scene();
Matrix get_world_matrix(Entity* entity);

template<typename T>
T* down_cast(Entity* entity) {
    return (T*)entity->specific_data;
}

#endif
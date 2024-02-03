#ifndef ENTITY_H
#define ENTITY_H

#include "obj_loader.h"
#include "matrix.h"
#include "render.h"
#include "physics.h"

#define CREATE_ENTITY(type, position, scale, mesh, material) (type*)create_entity_of_type(ENTITY_TYPE_##type, (u8*)pool_##type, &count_##type, sizeof(type), (position), (scale), (mesh), (material))

enum EntityType {
    ENTITY_TYPE_UNINITIALIZED,
    // These need to match the name of the entity type!
    ENTITY_TYPE_Camera,
    ENTITY_TYPE_Light,
    ENTITY_TYPE_Ground,
    ENTITY_TYPE_Player,
};

struct Entity {
    int id;
    EntityType type;
    Vec3 position;
    Vec3 scale;
    Matrix orientation;
    Model* model;
    Material* material;
    Collider collider;
    void* type_specific_data;
};
extern Entity entities[];
extern int entity_count;

struct Camera {
    Entity* entity;
    float yaw;
    float pitch;
    Vec3 target;
};

struct Light {
    Entity* entity;
    Vec3 ambient;
    Vec3 diffuse;
    Vec3 specular;
};

struct Ground {
    Entity* entity; //@Cleanup: This will probably be an ID instead of a pointer later.
};
extern Ground pool_Ground[];
extern int count_Ground;

struct Player {
    Entity* entity;
    Vec3 move;
    Vec3 velocity = {0,0,0};
    bool grounded = false;
    bool was_grounded = false;
    bool jumped_this_frame = false;
};
extern Player pool_Player[];
extern int count_Player;


extern Camera* main_camera;
extern Light* light;

extern Player* main_player;
extern Player* player2;

void reset_scene();
Matrix get_world_matrix(Entity* entity);

template<typename T>
T* down_cast(Entity* entity) {
    return (T*)entity->type_specific_data;
}

Ground* create_ground(Vec3 position, float width, float height, float depth);

AABB get_transformed_collider(Entity* entity);

#endif
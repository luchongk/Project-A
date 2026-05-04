#ifndef PHYSICS_H
#define PHYSICS_H

#include "maths.h"

struct Player;
struct Entity;

enum ColliderShape {
    COLLIDER_SHAPE_NONE,
    COLLIDER_SHAPE_BOX,
};

struct CollisionContact {
    Player* player;
    Entity* other_entity;
    Vector2 normal;
    float penetration;
};

struct AABB {
    Vector2 min;
    Vector2 max;
};

struct Collider {
    ColliderShape shape = COLLIDER_SHAPE_NONE;
    union {
        AABB box;
    };
};

bool collide_aabb_aabb(AABB* a, AABB* b, CollisionContact* contact);

#endif
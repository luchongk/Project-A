#ifndef PHYSICS_H
#define PHYSICS_H

#include "utils.h"
#include "vector.h"

struct Player;
struct Entity;

ENUM(ColliderShape,
    NONE,
    BOX
)

struct CollisionContact {
    Player* player;
    Entity* other_entity;
    Vec2 normal;
    float penetration;
};

struct AABB {
    Vec2 min;
    Vec2 max;
};

struct Collider {
    ColliderShape shape = ColliderShape::NONE;
    union {
        AABB box;
    };
};

bool collide_aabb_aabb(AABB* a, AABB* b, CollisionContact* contact);

#endif
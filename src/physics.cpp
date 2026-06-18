#include "physics.h"
#include "maths.h"

bool collide_aabb_aabb(AABB* a, AABB* b, CollisionContact* contact) {
    float overlap_left = b->max.x - a->min.x;
    if(overlap_left <= 0) return false;

    float overlap_right = a->max.x - b->min.x;
    if(overlap_right <= 0) return false;

    float overlap_down = b->max.y - a->min.y;
    if(overlap_down <= 0) return false;

    float overlap_up = a->max.y - b->min.y;
    if(overlap_up <= 0) return false;

    float overlap_x = min(overlap_left, overlap_right);
    float overlap_y = min(overlap_down, overlap_up);

    if(overlap_x < overlap_y) {
        contact->penetration = overlap_x;

        if(overlap_left < overlap_right) {
            contact->normal = Vector2{1, 0};
        }
        else {
            contact->normal = Vector2{-1, 0};
        }
    }
    else {
        contact->penetration = overlap_y;

        if(overlap_down < overlap_up) {
            contact->normal = Vector2{0, 1};
        }
        else {
            contact->normal = Vector2{0, -1};
        }
    }

    return true;
}

float raycast(Vector3 origin, Vector3 direction, AABB box) {
    //We need to do ray plane intersection and then bounds checking on the result.
    //The equation is: t = -(dot(n, origin) + k) / dot(n, direction)
    //Where n is the normal of the AABB and k is the distance to the origin.
    // t is the value to plug into the parametric of the ray r(t) = origin + direction * t.
    //Since we only have 2D AABBs, k = 0 and n = (0,0,1) for now.

    direction = normalize(direction);
    auto t = -dot(Vector3{0,0,1}, origin) / dot(Vector3{0,0,1}, direction);
    if(t < 0) return MAX_FLOAT;
    
    Vector2 hit_point = (Vector2)(origin + direction * t);
    if(hit_point.x < box.min.x || hit_point.x > box.max.x) return MAX_FLOAT;
    if(hit_point.y < box.min.y || hit_point.y > box.max.y) return MAX_FLOAT;
    
    return t;
}
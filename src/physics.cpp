#include "physics.h"
#include "maths.h"

bool collide_aabb_aabb(AABB* a, AABB* b, CollisionContact* contact) {
    float penetration_x_left = b->max.x - a->min.x;
    if(penetration_x_left < 0) return false;

    float penetration_x_right = a->max.x - b->min.x;
    if(penetration_x_right < 0) return false;

    float penetration_y_down = b->max.y - a->min.y;
    if(penetration_y_down < 0) return false;

    float penetration_y_up = a->max.y - b->min.y;
    if(penetration_y_up < 0) return false;

    float penetration_x = min(penetration_x_left, penetration_x_right);
    float penetration_y = min(penetration_y_down, penetration_y_up);

    if(penetration_x < penetration_y) {
        contact->penetration = penetration_x;

        if(penetration_x_left < penetration_x_right) {
            contact->normal = Vec2{1, 0};
        }
        else {
            contact->normal = Vec2{-1, 0};
        }
    }
    else {
        contact->penetration = penetration_y;

        if(penetration_y_down < penetration_y_up) {
            contact->normal = Vec2{0, 1};
        }
        else {
            contact->normal = Vec2{0, -1};
        }
    }

    return true;
}
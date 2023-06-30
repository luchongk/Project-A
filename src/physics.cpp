#include "utils.h"
#include "vector.h"

ENUM(ColliderType,
    BOX
)

struct BoxCollider {
    Vec2 offset;
    Vec2 size;
};

struct Collider {
    ColliderType type;
    union {
        BoxCollider box;
    };
};
#ifndef ENTITY_H
#define ENTITY_H

#include "base/arena.h"
#include "render.h"
#include "physics.h"

//#define CREATE_ENTITY(type, position, scale, mesh, material) (type*)create_entity_of_type(ENTITY_TYPE_##type, (u8*)pool_##type, &count_##type, sizeof(type), (position), (scale), (mesh), (material))

enum EntityType {
    ENTITY_TYPE_None,
    ENTITY_TYPE_Camera,
    ENTITY_TYPE_Light,
    ENTITY_TYPE_Player,
    ENTITY_TYPE_POOL_COUNT,
    
    // Types without pool.
    ENTITY_TYPE_Ground = ENTITY_TYPE_POOL_COUNT,
};

enum EntityFlags {
    ENTITY_FLAG_NONE,
    ENTITY_FLAG_ACTIVE,
    ENTITY_FLAG_MOUSE_PICKABLE
};

struct EntityHandle {
    u32 id;
    u32 generation;
};

struct Entity {
    EntityHandle handle;
    EntityType type;
    EntityFlags flags;
    
    Vector3 position;
    Vector3 scale;
    Matrix orientation;
    
    Model* model;
    Material* material;
    
    Collider collider;
    
    Entity* next_free;
    
    union {
        struct Camera* camera;
        struct Light* light;
        struct Player* player;
        void* type_specific_data;
    };
};

struct EntityTypeSpecific {
    Entity* entity;
};

struct Camera : EntityTypeSpecific {
    float yaw;
    float pitch;
    Vector3 forward;
    Vector2 target;
    float distance;
};

struct Light : EntityTypeSpecific {
    Vector3 ambient;
    Vector3 diffuse;
    Vector3 specular;
};

struct Player : EntityTypeSpecific {
    Vector3 move;
    Vector3 velocity = {0,0,0};
    bool grounded = false;
    bool was_grounded = false;
    bool jumped_this_frame = false;
};

const int entity_type_sizes[ENTITY_TYPE_POOL_COUNT] = {
    0,
    sizeof(Camera),
    sizeof(Light),
    sizeof(Player)
};

struct GenericPool {
    Arena* arena;
    void* data;
    u64 count;
};

template<typename T>
struct Pool {
    Arena* arena;
    ArrayView<T> all;
};

struct EntityManager {
    Pool<Entity> entities;
    Entity* entities_free_list;
    u32 entities_base;

    union {
        struct {
            GenericPool stub;
            Pool<Camera> cameras;
            Pool<Light> lights;
            Pool<Player> players;
        };
        GenericPool pools[ENTITY_TYPE_POOL_COUNT];
    };
};

template<typename T>
Pool<T> pool_init(Arena* arena) {
    assert(!arena->growable);
    
    Pool<T> p;
    p.arena = arena;
    p.all.data = (T*)((u8*)arena + arena->used);
    return p;
}

template<typename T>
T* pool_push(Pool<T>* pool, int count = 1, bool zero = true) {
    int to_alloc = (int)((pool->arena->used - sizeof(T) * count) / sizeof(T));
    arena_push<T>(pool->arena, to_alloc, alignof(T), zero);
    
    auto result = &pool->all[pool->all.count];
    pool->all.count += count;
    return result;
}

EntityManager* init_entity_manager();
void free_entity_manager(EntityManager* manager);
Entity* get_entity(EntityHandle handle);
EntityHandle create_entity(EntityType type, Vector3 position, Vector3 scale, Model* model, Material* material);
void destroy_entity(EntityHandle handle);
GenericPool pool_init(Arena* arena, int item_size);
void pool_reset(GenericPool* p);

EntityHandle create_camera(Vector3 position);
EntityHandle create_light(Vector3 position, Vector3 ambient, Vector3 diffuse, Vector3 specular);
EntityHandle create_player(Vector3 position, Vector3 scale, Model* model = &model_male);
EntityHandle create_ground(Vector3 position, Vector3 scale); 

void reset_scene();
Matrix get_world_matrix(Entity* entity);
AABB get_world_space_collider(Entity* entity);

const EntityHandle INVALID_ENTITY_HANDLE = {0,0};

extern EntityManager* manager;
extern EntityHandle main_camera;
extern EntityHandle light;
extern EntityHandle main_player;
extern EntityHandle player2;

#endif
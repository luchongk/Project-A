#include "entities.h"
#include "simulation.h"
#include "base/temp_allocator.h"

EntityManager* manager;

EntityHandle main_camera;
EntityHandle light;
EntityHandle main_player;
EntityHandle player2;

EntityManager* init_entity_manager() {
    Arena* arena = arena_alloc(GIGABYTES(16), KILOBYTES(64), false);
    manager = arena_push<EntityManager>(arena);
    manager->entities_base = (u32)arena->used;
    manager->entities = pool_init<Entity>(arena);

    for(int i = 1; i < ENTITY_TYPE_POOL_COUNT; i++) {
        manager->pools[i] = pool_init(arena_alloc(GIGABYTES(1), KILOBYTES(64), false), entity_type_sizes[i]);
    }
    return manager;
}

void free_entity_manager(EntityManager* manager) {
    for(int i = 1; i < ENTITY_TYPE_POOL_COUNT; i++) {
        arena_free(manager->pools[i].arena);
    }
    arena_free(manager->entities.arena);
}

Entity* get_entity(EntityHandle handle) {
    if(handle.id == INVALID_ENTITY_HANDLE.id || handle.id >= manager->entities.all.count) return nullptr;
    auto entity = &manager->entities.all[handle.id];
    if(!(entity->flags & ENTITY_FLAG_ACTIVE) || handle.generation != entity->handle.generation) return nullptr;
    return entity;
}

Entity* alloc_entity(EntityType type, Vector3 position, Vector3 scale, Model* model, Material* material) {
    Entity* e = manager->entities_free_list;
    if(e) {
        manager->entities_free_list = e->next_free;
    } else {
        e = arena_push<Entity>(manager->entities.arena);
        e->handle.id = (u32)manager->entities.all.count++;
        e->handle.generation = 0;
    }

    e->type        = type;
    e->flags       = (EntityFlags)(ENTITY_FLAG_ACTIVE | ENTITY_FLAG_MOUSE_PICKABLE);
    e->position    = position;
    e->scale       = scale;
    e->orientation = Matrix::ident;
    e->model       = model;
    e->material    = material;
    return e;
}

EntityHandle create_entity(EntityType type, Vector3 position, Vector3 scale, Model* model, Material* material) {
    Entity* e = alloc_entity(type, position, scale, model, material);
    return e->handle;
}

void destroy_entity(EntityHandle handle) {
    auto entities = manager->entities.all;
    assert(handle.id < entities.count);
    auto e = &entities[handle.id];

    if(!(e->flags & ENTITY_FLAG_ACTIVE) || handle.generation != e->handle.generation) return; // Already deleted.
    
    e->flags = ENTITY_FLAG_NONE;
    e->handle.generation++;
    auto next = manager->entities_free_list;
    manager->entities_free_list = e;
    e->next_free = next;

    if(e->type < ENTITY_TYPE_POOL_COUNT) {
        auto subtype_pool = &manager->pools[e->type];
        auto start = subtype_pool->data;
        auto size  = entity_type_sizes[e->type];
        auto index = ((u8*)e->type_specific_data - (u8*)start) / size;
        auto to_destroy = (u8*)start + size * index;
        auto last       = (u8*)start + size * (subtype_pool->count - 1);
        ((EntityTypeSpecific*)last)->entity->type_specific_data = to_destroy;
        memcpy(to_destroy, last, size);
        subtype_pool->count--;
    }
}

GenericPool pool_init(Arena* arena, int item_size) {
    assert(!arena->growable);
    
    GenericPool p;
    p.arena = arena;
    p.data = ((u8*)arena + arena->used);
    return p;
}

void pool_reset(GenericPool* p) {
    arena_clear(p->arena);
    p->count = 0;
}

EntityHandle create_camera(Vector3 position) {
    auto camera = pool_push<Camera>(&manager->cameras);
    auto entity = alloc_entity(ENTITY_TYPE_Camera, position, Vector3{1,1,1}, nullptr, nullptr);
    entity->flags = (EntityFlags)(entity->flags & ~ENTITY_FLAG_MOUSE_PICKABLE);
    camera->entity = entity;
    camera->yaw   = radians(0);
    camera->pitch = radians(-25);
    camera->target = camera->entity->position.xy;
    camera->distance = 10;
    entity->camera = camera;
    return entity->handle;
}

EntityHandle create_light(Vector3 position, Vector3 ambient, Vector3 diffuse, Vector3 specular) {
    auto light = pool_push<Light>(&manager->lights);
    auto entity = alloc_entity(ENTITY_TYPE_Light, position, Vector3{.1f,.1f,.1f}, &model_cube, &MATERIAL_LIGHT);
    entity->flags = (EntityFlags)(entity->flags & ~ENTITY_FLAG_MOUSE_PICKABLE);
    light->entity = entity;
    light->ambient  = ambient;
    light->diffuse  = diffuse;
    light->specular = specular;
    entity->light = light;
    return entity->handle;
}

EntityHandle create_player(Vector3 position, Vector3 scale, Model* model) {
    auto player = pool_push<Player>(&manager->players);
    auto entity = alloc_entity(ENTITY_TYPE_Player, position, scale, model, &MATERIAL_PLAYER);
    player->entity = entity;
    player->velocity = {};
    entity->collider.shape = COLLIDER_SHAPE_BOX;
    entity->collider.box.min = -(Vector2)scale / 2;
    entity->collider.box.max =  (Vector2)scale / 2;
    entity->player = player;
    return entity->handle;
}

EntityHandle create_ground(Vector3 position, Vector3 scale) {
    auto entity = alloc_entity(ENTITY_TYPE_Ground, position, scale, &model_cube, &MATERIAL_GROUND);
    entity->collider.shape = COLLIDER_SHAPE_BOX;
    entity->collider.box.min = -(Vector2)entity->scale / 2;
    entity->collider.box.max = (Vector2)entity->scale / 2;
    return entity->handle;
}

void reset_scene() {
    auto old_camera = get_entity(main_camera);
    Vector3 camera_pos = Vector3{-6, 5, 7};
    Camera* camera = tnew<Camera>();
    if(old_camera) {
        camera_pos = old_camera->position;
        *camera = *old_camera->camera;
    }

    arena_pop(manager->entities.arena, manager->entities_base);
    arena_push<Entity>(manager->entities.arena);
    manager->entities.all.count = 1; // Invalid entity
    manager->entities_free_list = nullptr;
    for(int i = 1; i < ENTITY_TYPE_POOL_COUNT; i++) {
        pool_reset(&manager->pools[i]);
    }

    main_camera = create_camera(camera_pos);
    if(old_camera) {
        auto c = get_entity(main_camera);
        *c->camera = *camera;
    }

    light = create_light(Vector3{-2, 3, 3}, Vector3{0.0005f, 0.0005f, 0.0005f}, Vector3{1.0f, 1.0f, 1.0f}, Vector3{1.0f, 1.0f, 1.0f});

    main_player = create_player(Vector3{0, 0.5f, 0}, Vector3{0.65f, 1.8f, 1});

    player2 = create_player(Vector3{3, 5.0f, 0}, Vector3{0.65f, 1.8f, 1});

    auto player3 = create_player(Vector3{0, 10.0f, 0}, Vector3{0.65f, 1.7f, 1}, &model_female);
    
    auto player4 = create_player(Vector3{0, 15.0f, 0}, Vector3{0.65f, 1.7f, 1}, &model_female);

    create_ground({0, -0.5f, 0},  Vector3{1000, 1, 3});
    create_ground({0, 15.0f, 0},  Vector3{20, 1, 3});
    create_ground({-10, 7.5f, 0}, Vector3{ 1, 15, 3});
}

// @Speed: Can we calculate world-view matrix at once or do we need them to be separate?
Matrix get_world_matrix(Entity* entity) {
    Matrix localToWorld = entity->orientation;
    localToWorld = scale(localToWorld, entity->scale);
    localToWorld = translate(localToWorld, entity->position);
    
    return localToWorld;
}

AABB get_world_space_collider(Entity* entity) {
    AABB aabb;
    aabb.min = (Vector2)entity->position + entity->collider.box.min;
    aabb.max = (Vector2)entity->position + entity->collider.box.max;

    return aabb;
}
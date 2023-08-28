#include "entities.h"

const int MAX_ENTITIES = 100;

Entity entities[MAX_ENTITIES];
int entity_count = 0;

Ground pool_Ground[MAX_ENTITIES];
int count_Ground = 0;

Camera pool_Camera[MAX_ENTITIES];
int count_Camera = 0;

Light pool_Light[MAX_ENTITIES];
int count_Light = 0;

Player pool_Player[MAX_ENTITIES];
int count_Player = 0;

Camera* main_camera;
Light* light;
Player* main_player;
Player* player2;

Entity* create_entity(Vec3 position, float scale, Mesh* mesh, Material* material) {
    Entity* e = &entities[entity_count++];
    e->type        = EntityType::UNINITIALIZED;
    e->scale       = scale;
    e->orientation = Matrix::ident;
    e->position    = position;
    e->mesh        = mesh;
    e->material    = material;

    return e;
}

inline void* create_entity_of_type(EntityType type, u8* pool, int* count, int size, Vec3 position, float scale, Mesh* mesh, Material* material) {
    void* specific = pool + size * (*count);
    (*count)++;
    
    auto e = create_entity(position, scale, mesh, material);
    e->type = type;
    e->type_specific_data = specific;
    
    *((Entity**)specific) = e;
    return specific;
}

Ground* create_ground(Vec3 position, float width, float height, float depth) {
    auto ground = CREATE_ENTITY(Ground, position, 1, &cube_mesh, &MATERIAL_GROUND);
    ground->dimensions = Vec3{width, height, depth};
    ground->entity->collider.shape = ColliderShape::BOX;
    ground->entity->collider.box.min = -vec2(ground->dimensions) / 2;
    ground->entity->collider.box.max = vec2(ground->dimensions) / 2;
    return ground;
}

Player* create_player(Vec3 position, float width, float height, float depth) {
    auto player = CREATE_ENTITY(Player, position, 1, &cube_mesh, &MATERIAL_PLAYER);
    player->dimensions = Vec3{width, height, depth};
    player->entity->collider.shape = ColliderShape::BOX;
    player->entity->collider.box.min = -vec2(player->dimensions) / 2;
    player->entity->collider.box.max = vec2(player->dimensions) / 2;
    return player;
}

void reset_scene() {
    cubes_rotation = 0;
    entity_count = 0;
    count_Camera = 0;
    count_Light = 0;
    count_Ground = 0;
    count_Player = 0;
    do_the_thing_cooldown = 0;

    main_camera = CREATE_ENTITY(Camera, (Vec3{-6, 5, 7}), 1, nullptr, nullptr);
    main_camera->yaw = to_radians(180);
    main_camera->pitch = to_radians(-25);
    main_camera->entity->type_specific_data = &main_camera;

    light = CREATE_ENTITY(Light, (Vec3{-2, 3, 3}), 0.1f, &cube_mesh, &MATERIAL_LIGHT);
    light->ambient  = {0.0005f, 0.0005f, 0.0005f};
    light->diffuse  = {1.0f, 1.0f, 1.0f};
    light->specular = {1.0f, 1.0f, 1.0f};
    light->entity->type_specific_data = &light;

    main_player = create_player({0, 0.5f, 0}, 1, 2, 1);

    player2 = create_player({0, 5.0f, 0}, 1, 2, 1);
    player2->entity->material = &MATERIAL_PLAYER2;

    auto player3 = create_player({2, 5.0f, 0}, 1, 2, 1);
    player3->entity->material = &MATERIAL_PLAYER2;

    create_ground({0, -0.5f, 0}, 20, 1, 3);
    create_ground({0, 15.0f, 0}, 20, 1, 3);
    create_ground({-10, 7.5f, 0}, 1, 15, 3);
}

// @Speed: Can we calculate world-view matrix at once or do we need them to be separate?
Matrix get_world_matrix(Entity* entity) {
    Matrix localToWorld = entity->orientation;

    if(entity->type == EntityType::Ground || entity->type == EntityType::Player) { //@Hack: Fix this stupidness.
        auto ground = (Ground*)entity->type_specific_data;
        localToWorld = scale(localToWorld, entity->scale * ground->dimensions);
    }
    else {
        localToWorld = scale(localToWorld, entity->scale);
    }

    localToWorld = translate(localToWorld, entity->position);

    return localToWorld;
}

AABB get_transformed_collider(Entity* entity) {
    AABB aabb;
    aabb.min = vec2(entity->position) + entity->collider.box.min;
    aabb.max = vec2(entity->position) + entity->collider.box.max;

    return aabb;
}
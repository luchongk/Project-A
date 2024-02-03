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

Entity* create_entity(Vec3 position, Vec3 scale, Model* model, Material* material) {
    Entity* e = &entities[entity_count++];
    e->type        = ENTITY_TYPE_UNINITIALIZED;
    e->scale       = scale;
    e->orientation = Matrix::ident;
    e->position    = position;
    e->model       = model;
    e->material    = material;

    return e;
}

inline void* create_entity_of_type(EntityType type, u8* pool, int* count, int size, Vec3 position, Vec3 scale, Model* model, Material* material) {
    void* specific = pool + size * (*count);
    (*count)++;
    
    auto e = create_entity(position, scale, model, material);
    e->type = type;
    e->type_specific_data = specific;
    
    *((Entity**)specific) = e;
    return specific;
}

Ground* create_ground(Vec3 position, float width, float height, float depth) {
    auto scale = Vec3{width, height, depth};
    auto ground = CREATE_ENTITY(Ground, position, scale, &model_cube, &MATERIAL_GROUND);
    ground->entity->collider.shape = COLLIDER_SHAPE_BOX;
    ground->entity->collider.box.min = -vec2(ground->entity->scale) / 2;
    ground->entity->collider.box.max = vec2(ground->entity->scale) / 2;
    return ground;
}

Player* create_player(Vec3 position, float width, float height, float depth, Model* model = &model_male) {
    auto scale = Vec3{1, 1, 1};
    auto player = CREATE_ENTITY(Player, position, scale, model, &MATERIAL_PLAYER);
    player->entity->collider.shape = COLLIDER_SHAPE_BOX;
    player->entity->collider.box.min = -Vec2{width, height} / 2;
    player->entity->collider.box.max = Vec2{width, height} / 2;
    player->velocity = {};
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

    main_camera = CREATE_ENTITY(Camera, (Vec3{-6, 5, 7}), (Vec3{1,1,1}), nullptr, nullptr);
    main_camera->yaw   = radians(0);
    main_camera->pitch = radians(-25);
    main_camera->target = main_camera->entity->position;
    main_camera->entity->type_specific_data = &main_camera;

    light = CREATE_ENTITY(Light, (Vec3{-2, 3, 3}), 0.1f * (Vec3{1,1,1}), &model_cube, &MATERIAL_LIGHT);
    light->ambient  = {0.0005f, 0.0005f, 0.0005f};
    light->diffuse  = {1.0f, 1.0f, 1.0f};
    light->specular = {1.0f, 1.0f, 1.0f};
    light->entity->type_specific_data = &light;

    main_player = create_player({0, 0.5f, 0}, 0.65f, 1.8f, 1);

    player2 = create_player({3, 5.0f, 0}, 0.65f, 1.8f, 1, &model_male);

    /*auto player3 = */create_player({0, 10.0f, 0}, 0.65f, 1.7f, 1, &model_female);
    
    /*auto player4 = */create_player({0, 15.0f, 0}, 0.65f, 1.7f, 1, &model_female);

    create_ground({0, -0.5f, 0}, 1000, 1, 3);
    create_ground({0, 15.0f, 0}, 20, 1, 3);
    create_ground({-10, 7.5f, 0}, 1, 15, 3);
}

// @Speed: Can we calculate world-view matrix at once or do we need them to be separate?
Matrix get_world_matrix(Entity* entity) {
    Matrix localToWorld = entity->orientation;

    localToWorld = scale(localToWorld, entity->scale);
    localToWorld = translate(localToWorld, entity->position);

    return localToWorld;
}

AABB get_transformed_collider(Entity* entity) {
    AABB aabb;
    aabb.min = vec2(entity->position) + entity->collider.box.min;
    aabb.max = vec2(entity->position) + entity->collider.box.max;

    return aabb;
}
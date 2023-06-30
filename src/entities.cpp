#include "entities.h"

Entity entities[MAX_ENTITIES];
int entity_count = 0;

Camera* main_camera;
Light* light;
Ground* player;

EntityType EntityPool<Camera>::type = EntityType::CAMERA;
EntityType EntityPool<Light>::type  = EntityType::LIGHT;
EntityType EntityPool<Ground>::type = EntityType::GROUND;

static Entity* make_entity(EntityType type, Vec3 position, float scale, Mesh* mesh, Material* material) {
    Entity* e = &entities[entity_count];
    e->type        = type;
    e->scale       = scale;
    e->orientation = Matrix::ident;
    e->position    = position;
    e->mesh        = mesh;
    e->material    = material;

    ++entity_count;

    return e;
}

template<typename T>
static T* make_entity(Vec3 position, float scale, Mesh* mesh, Material* material) {
    auto e = make_entity(EntityPool<T>::type, position, scale, mesh, material);
    T* specific = &EntityPool<T>::pool[EntityPool<T>::count++];
    specific->entity = e;
    e->specific_data = specific;

    return specific;
}

static Ground* make_ground(Vec3 position, float width, float height, float depth) {
    auto ground = make_entity<Ground>(position, 1, &cube_mesh, &MATERIAL_GROUND);
    ground->dimensions = Vec3{width, height, depth};
    return ground;
}

void reset_scene() {
    cubes_rotation = 0;
    entity_count = 0;

    main_camera = make_entity<Camera>({0, 5, 7}, 1, nullptr, nullptr);
    main_camera->yaw = to_radians(180);
    main_camera->pitch = to_radians(-25);
    main_camera->entity->specific_data = &main_camera;

    light = make_entity<Light>({-2, 3, 3}, 0.1f, &cube_mesh, &MATERIAL_LIGHT);
    light->ambient  = {0.0005f, 0.0005f, 0.0005f};
    light->diffuse  = {1.0f, 1.0f, 1.0f};
    light->specular = {1.0f, 1.0f, 1.0f};
    light->entity->specific_data = &light;

    make_ground({0, 0, 0}, 20, 1, 3);
    auto origin = make_ground({0, 0, 0}, 1.1f, 1.1f, 1.1f);
    origin->entity->material = &MATERIAL_GROUND2;

    player = make_ground({0, 5.0f, 0}, 1, 2, 1);
    player->entity->material = &MATERIAL_GROUND2;
}

// @Speed: Can we calculate world-view matrix at once or do we need them to be separate?
Matrix get_world_matrix(Entity* entity) {
    Matrix localToWorld = entity->orientation;

    if(entity->type == EntityType::GROUND) {
        auto ground = (Ground*)entity->specific_data;
        localToWorld = scale(localToWorld, entity->scale * ground->dimensions);
    }
    else {
        localToWorld = scale(localToWorld, entity->scale);
    }

    localToWorld = translate(localToWorld, entity->position);

    return localToWorld;
}
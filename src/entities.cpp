#include "entities.h"

Camera camera;
Light light;
Entity entities[10];
int entities_count = 0;

static void make_entity(Vec3 position) {
    Entity* e = &entities[entities_count];
    e->scale = 2.0f;
    e->orientation = Matrix::ident;
    e->position = position;
    e->mesh = &weird_mesh;
    e->material.ambient   = {0.1f, 0.07f, 0.01f};
    e->material.diffuse   = {0.05f, 0.025f, 0.005f};
    e->material.specular  = {1.0f, 0.7f, 0.0f};
    //e->material.specular  = {0.0f, 0.0f, 0.0f};
    e->material.shininess = 32.0f;

    ++entities_count;
}

void reset_scene() {
    camera.position = {-7.0f, 3.5f, -6.0f};
    camera.pitch = 0.403062f;
    camera.yaw = 2.05159f;

    light = {};
    light.position = {0, 0, 0};
    light.scale = .05f;
    light.orientation = Matrix::ident;
    light.diffuse  = {1.0f, 1.0f, 1.0f};
    light.ambient  = {0.03f, 0.03f, 0.03f};
    light.specular = {1.0f, 1.0f, 1.0f};
    light.mesh = &cube_mesh;

    cubes_rotation = 0;
    entities_count = 0;
    make_entity({0.0f, 0.0f, 0.0f});
    //make_entity({2.0f, 5.0f, -15.0f});
    //make_entity({-1.5f, -2.2f, -2.5f});
    //make_entity({-3.8f, -2.0f, -12.3f});
    //make_entity({2.4f, -0.4f, -3.5f});
}

Matrix get_world_matrix(Entity* entity) {
    Matrix localToWorld = translation(entity->position);
    localToWorld = scale(localToWorld, entity->scale);
    localToWorld *= entity->orientation;

    return localToWorld;
}
#include "entities.h"

Camera camera;
Light light;
Entity entities[10];

void reset_entities() {
    camera.position = {0,0,2};

    light = {};
    light.scale = 1.0f;
    light.orientation = Matrix::ident;
    light.diffuse = {1.0f, 1.0f, 1.0f};
    light.ambient = per_frame_uniforms.light.diffuse * Vec3{0.03f, 0.03f, 0.03f};
    light.specular = {1.0f, 1.0f, 1.0f};
    light.mesh = &cube_mesh;

    for(int i = 0; i < 5; i++) {
        entities[i].scale = 0.5f;
        entities[i].orientation = Matrix::ident;
    }

    entities[0].position = {0.0f, 0.0f, 0.0f};
    entities[0].mesh = &weird_mesh;
    entities[0].material.ambient   = {0.49f, 0.37f, 0.11f};
    entities[0].material.diffuse   = {0.5f, 0.35f, 0.05f};
    entities[0].material.specular  = {1.0f, 0.782f, 0.17f};
    entities[0].material.shininess = 16.0f;
    
    entities[1].position = {2.0f, 5.0f, -15.0f};
    entities[1].mesh = &weird_mesh;
    entities[1].material.ambient   = {0.49f, 0.37f, 0.11f};
    entities[1].material.diffuse   = {0.5f, 0.35f, 0.05f};
    entities[1].material.specular  = {1.0f, 0.782f, 0.17f};
    entities[1].material.shininess = 16.0f;
    
    entities[2].position = {-1.5f, -2.2f, -2.5f};
    entities[2].mesh = &weird_mesh;
    entities[2].material.ambient   = {0.49f, 0.37f, 0.11f};
    entities[2].material.diffuse   = {0.5f, 0.35f, 0.05f};
    entities[2].material.specular  = {1.0f, 0.782f, 0.17f};
    entities[2].material.shininess = 16.0f;
    
    entities[3].position = {-3.8f, -2.0f, -12.3f};
    entities[3].mesh = &weird_mesh;
    entities[3].material.ambient   = {0.49f, 0.37f, 0.11f};
    entities[3].material.diffuse   = {0.5f, 0.35f, 0.05f};
    entities[3].material.specular  = {1.0f, 0.782f, 0.17f};
    entities[3].material.shininess = 16.0f;
    
    entities[4].position = {2.4f, -0.4f, -3.5f};
    entities[4].mesh = &weird_mesh;
    entities[4].material.ambient   = {0.49f, 0.37f, 0.11f};
    entities[4].material.diffuse   = {0.5f, 0.35f, 0.05f};
    entities[4].material.specular  = {1.0f, 0.782f, 0.17f};
    entities[4].material.shininess = 16.0f;
}

Matrix get_world_matrix(Entity* entity) {
    Matrix localToWorld = translation(per_object_uniforms.position);
    localToWorld = scale(localToWorld, entity->scale);
    localToWorld *= entity->orientation;

    return localToWorld;
}
#include "entities.h"
#include "ui.h"

//All these are @TEMPORARY
static float cubes_rotation_dir = 1;

float cubes_rotation = 0;
bool paused = false;
bool character_selected = true;

static void update_camera(float dt) {
    float yaw = input.mouse_delta_pixels.x * radians(2.0f) * dt;
    camera.yaw = normalize_angle(camera.yaw + yaw);
    
    float pitch = input.mouse_delta_pixels.y * radians(2.0f) * dt;
    camera.pitch = normalize_angle(camera.pitch + pitch);

    camera.forward = angles_to_vec(camera.yaw, camera.pitch);

    if(character_selected) {
        if(input.move.x != 0) {
            camera.position += normalize(cross(Vec3{0,1,0}, camera.forward)) * 3.0f * input.move.x * dt;
        }

        if(input.move.z != 0) {
            camera.position -= camera.forward * 2.0f * input.move.z * dt;
        }

        if(input.move.y != 0) {
            camera.position += Vec3{0,1,0} * 2.0f * input.move.y * dt;
        }
    }
}

void simulate(float dt) {
    ui_update();

    if(paused) return;

    float non_scaled_dt = dt;
    dt = dt * time.sim_scale;

    update_camera(non_scaled_dt);
    /*std::cout << "POS: " << camera.position.x << " " << camera.position.y << " " << camera.position.z << std::endl;
    std::cout << "PITCH: " << camera.pitch << " YAW:" << camera.yaw << std::endl;*/
    
    cubes_rotation += -input.rotation * radians(45.0f) * dt;
    
    //per_object_uniforms.material.diffuse += Vec3{0.0f, 0.0f, input->vertical * dt};
    Vec3 move = input.move;
    for(int i = 0; i < 5; i++) {
        Entity* e = &entities[i];
        if(!character_selected) {
            e->position.x += move.x * 3.0f * dt;
            e->position.y += move.y * 3.0f * dt;
            e->position.z -= move.z * 3.0f * dt;
        }
        e->orientation = rotation(cubes_rotation, cubes_rotation);
    }

    /*light.position = {
        cosf(time.since_start * 0.75f) * 2,
        sinf(time.since_start * 0.75f) * 2,
        -6.0f
    };*/
}
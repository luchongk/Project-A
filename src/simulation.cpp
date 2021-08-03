#include "entities.h"

//All these are @TEMPORARY
static float cubes_rotation_dir = 1;

float cubes_rotation = 0;
bool paused = false;
bool character_selected = false;

static void update_camera(float dt) {
    float yaw = player_input.mouse_delta.x * radians(2.0f) * dt;
    camera.yaw = normalize_angle(camera.yaw + yaw);
    
    float pitch = player_input.mouse_delta.y * radians(2.0f) * dt;
    camera.pitch = normalize_angle(camera.pitch + pitch);

    camera.forward = angles_to_vec(camera.yaw, camera.pitch);

    if(character_selected) {
        if(player_input.move.x != 0) {
            camera.position += normalize(cross(Vec3{0,1,0}, camera.forward)) * 3.0f * player_input.move.x * dt;
        }

        if(player_input.move.z != 0) {
            camera.position -= camera.forward * 2.0f * player_input.move.z * dt;
        }

        if(player_input.move.y != 0) {
            camera.position += Vec3{0,1,0} * 2.0f * player_input.move.y * dt;
        }
    }
}

void simulate(float dt) {
    if(paused) return;
    
    float non_scaled_dt = dt;
    dt = dt * time.sim_scale;

    update_camera(non_scaled_dt);
    
    //if(!paused) {
        light.t_movement += dt;
    //}
    
    if(!character_selected) {
        cubes_rotation += -player_input.rotation * radians(45.0f) * dt;
        //per_object_uniforms.material.diffuse += Vec3{0.0f, 0.0f, input->vertical * dt};
        for(int i = 0; i < 5; i++) {
            entities[i].position.x += player_input.move.x * 3.0f * dt;
            entities[i].position.y += player_input.move.y * 3.0f * dt;
            entities[i].position.z -= player_input.move.z * 3.0f * dt;
            entities[i].orientation = rotation(cubes_rotation, cubes_rotation);
        }
    }
    //cubes_rotation += 2 * sinf(10 * light.t_movement) * cubes_rotation_dir * dt;
    light.position = {
        cosf(light.t_movement * 0.75f) * 2,
        sinf(light.t_movement * 0.75f) * 2,
        -6.0f
    };
}
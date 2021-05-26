#include "entities.h"

//All these are @TEMPORARY
static float cubes_rotation_dir = 1;

float cubes_rotation = 0;
bool paused = false;
bool character = false;

static void update_camera(float dt) {
    float yaw = input.mouse_delta.x * radians(2.0f) * dt;
    camera.yaw = normalize_angle(camera.yaw + yaw);
    
    float pitch = input.mouse_delta.y * radians(2.0f) * dt;
    camera.pitch = normalize_angle(camera.pitch + pitch);

    camera.forward = angles_to_vec(camera.yaw, camera.pitch);

    if(character) {
        if(key_axis('A', 'D') != 0) {
            camera.position += normalize(cross(Vec3{0,1,0}, camera.forward)) * 3.0f * (float)key_axis('A', 'D') * dt;
        }

        if(key_axis('S', 'W') != 0) {
            camera.position -= camera.forward * 2.0f * (float)key_axis('S', 'W') * dt;
        }

        if(key_axis(VK_SHIFT, VK_SPACE) != 0) {
            camera.position += Vec3{0,1,0} * 2.0f * (float)key_axis(VK_SHIFT, VK_SPACE) * dt;
        }
    }
}

void simulate(float dt) {
    if(paused) return;

    update_camera(dt);
    
    light.t_movement += dt;
    cubes_rotation += -key_axis('Q', 'E') * radians(45.0f) * dt;
    if(!character) {
        //per_object_uniforms.material.diffuse += Vec3{0.0f, 0.0f, input->vertical * dt};
        for(int i = 0; i < 5; i++) {
            entities[i].position.x += key_axis('A', 'D') * 3.0f * dt;
            entities[i].position.y += key_axis(VK_SHIFT, VK_SPACE)    * 3.0f * dt;
            entities[i].position.z -= key_axis('S', 'W') * 3.0f * dt;
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
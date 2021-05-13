#include "entities.h"

//All these are @TEMPORARY
static float cubes_rotation_dir = 1;

Vec3 light_pos = {0.0f, 0.0f, 0.0f};
float cubes_rotation = 0;
Vec3 cubes_offset;
bool paused = false;
bool character = false;
float light_time_accum = 0.0f;

static void update_camera(PlayerInput* input, float dt) {
    float yaw = input->mouse_delta.x * radians(2.0f) * dt;
    camera.yaw = clamp_angle(camera.yaw + yaw);
    
    float pitch = input->mouse_delta.y * radians(2.0f) * dt;
    camera.pitch = clamp_angle(camera.pitch + pitch);

    camera.forward = from_angles(camera.yaw, camera.pitch);

    if(character && input->horizontalX != 0) {
        camera.position += normalize(cross(Vec3{0,1,0}, camera.forward)) * 3.0f * (float)input->horizontalX * dt;
    }

    if(character && input->horizontalZ != 0) {
        camera.position -= camera.forward * 2.0f * (float)input->horizontalZ * dt;
    }

    if(character && input->vertical != 0) {
        camera.position += Vec3{0,1,0} * 2.0f * (float)input->vertical * dt;
    }
}

void simulate(PlayerInput* input) {
    if(paused) return;

    float dt = time.delta * time.modifier;

    update_camera(input, dt);
    
    light_time_accum += (float)time.delta;
    if(!character) {
        //per_object_uniforms.material.diffuse += Vec3{0.0f, 0.0f, input->vertical * dt};
        cubes_offset.x += input->horizontalX * 3.0f * dt;
        cubes_offset.y += input->vertical    * 3.0f * dt;
        cubes_offset.z -= input->horizontalZ * 3.0f * dt;
    }
    cubes_rotation += -input->rotationDir * radians(45.0f) * dt;
    //cubes_rotation += 2 * sinf(10 * light_time_accum) * cubes_rotation_dir * dt;
    light_pos = {
        cosf(light_time_accum * 0.75f) * 2,
        sinf(light_time_accum * 0.75f) * 2,
        -6.0f
    };
}
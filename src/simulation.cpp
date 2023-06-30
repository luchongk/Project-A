#include "entities.h"
#include "ui.h"

//All these are @TEMPORARY
static float cubes_rotation_dir = 1;

float cubes_rotation = 0;
bool paused = false;
bool character_selected = true;
float camera_distance = 10.0f;

static void update_camera(float dt) {
    float delta_yaw = -input.mouse_delta_pixels.x * to_radians(2.0f) * dt;
    main_camera->yaw = normalize_angle(main_camera->yaw + delta_yaw);
    
    float delta_pitch = input.mouse_delta_pixels.y * to_radians(2.0f) * dt;
    main_camera->pitch = clamp(main_camera->pitch + delta_pitch, to_radians(-89), to_radians(89));

    Vec3 forward = angles_to_vec(main_camera->yaw, main_camera->pitch);

    if(!character_selected) {
        if(input.move.x != 0) {
            main_camera->entity->position += normalize(cross(Vec3{0,1,0}, -forward)) * 3.0f * input.move.x * dt;
        }

        if(input.move.y != 0) {
            main_camera->entity->position += Vec3{0,1,0} * 2.0f * input.move.y * dt;
        }

        if(input.move.z != 0) {
            main_camera->entity->position += forward * 2.0f * input.move.z * dt;
        }
    }
    else {
        float zoom = 10.0;
        main_camera->entity->position = player->entity->position + normalize(Vec3{0, 4, 7}) * camera_distance;
    }

    per_frame_uniforms.view_pos = main_camera->entity->position;
    per_frame_uniforms.view = look_to(main_camera->entity->position, forward);
}

Vec3 player_velocity = {};
const Vec3 g_acceleration = {0, 2 * -9.8f, 0};
bool grounded = false;
float move_speed = 5;
float jump_speed = 10;

void simulate(float dt) {
    ui_update();
    if(paused) return;

    float non_scaled_dt = dt;
    dt = dt * my_time.sim_scale;
   
    cubes_rotation += -input.rotation * to_radians(45.0f) * dt;
    
    {
        if(grounded) {
            player_velocity.x = 0;
            player_velocity.z = 0;
        }
        player_velocity += g_acceleration * dt;
        if(character_selected) {
            float actual_move_seed = move_speed;
            if(grounded) {
                if(input.move.y == 1) {
                    player_velocity.y += jump_speed;
                    grounded = false;
                }
            } else {
                actual_move_seed = 0.005f * move_speed; // Air move speed.
            }
            player_velocity += actual_move_seed * normalize(Vec3{input.move.x, 0, -input.move.z});
        }

        // Integrate velocity
        player->entity->position += player_velocity * dt;

        // Handle collisions
        if(player->entity->position.y < 1.5f) {
            player_velocity.y = 0;
            player->entity->position.y = 1.5f;
            grounded = true;
        }
    }

    update_camera(non_scaled_dt);

    /*for(int i = 0; i < count_Ground; i++) {
        Entity* e = grounds[i].entity;
        e->orientation = rotation(cubes_rotation, cubes_rotation);
        e->scale = abs(sin(my_time.since_start));
    }*/
}
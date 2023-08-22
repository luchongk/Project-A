#include "entities.h"
#include "ui.h"

//All these are @TEMPORARY
static float cubes_rotation_dir = 1;

float cubes_rotation = 0;
bool paused = false;
bool character_selected = true;
float camera_distance_target = 10;
float camera_distance = 10;
float zoom_sensitivity = 2.5f;
float zoom_snappiness = 10.0f;

// Player movement
const Vec3 g_acceleration = {0, 3 * -9.8f, 0};
float grounded_speed = 3;
float jump_speed = 12;

static void update_camera(float dt) {
    float delta_yaw = -input.mouse_delta_pixels.x * to_radians(2.0f) * dt;
    main_camera->yaw = normalize_angle(main_camera->yaw + delta_yaw);
    
    float delta_pitch = input.mouse_delta_pixels.y * to_radians(2.0f) * dt;
    main_camera->pitch = clamp(main_camera->pitch + delta_pitch, to_radians(-89), to_radians(89));

    Vec3 forward = angles_to_vec(main_camera->yaw, main_camera->pitch);

    Vec3 final_camera_position;
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

        final_camera_position = main_camera->entity->position;
    }
    else {        
        main_camera->entity->position.x = exp_interpolate(main_camera->entity->position.x, main_player->entity->position.x, dt);
        main_camera->entity->position.y = exp_interpolate(main_camera->entity->position.y, main_player->entity->position.y, dt);
        main_camera->entity->position.z = main_player->entity->position.z;

        static auto last_player_pos = 0.0f;
        auto player_dx = (main_player->entity->position.x - last_player_pos) / dt;
        last_player_pos = main_player->entity->position.x;
        camera_distance_target = fabs(player_dx) * 8;
        camera_distance_target = clamp<float>(camera_distance_target, 10, 25);
        camera_distance = exp_interpolate(camera_distance, camera_distance_target, dt, 0.1f);
        final_camera_position = main_camera->entity->position + normalize(Vec3{0, 4, 7}) * camera_distance;
    }

    per_frame_uniforms.view_pos = final_camera_position;
    per_frame_uniforms.view = look_to(final_camera_position, forward);
}

float do_the_thing_cooldown = 0;
void simulate(float dt) {
    {
        cubes_rotation += -input.rotation * to_radians(45.0f) * dt;
        
        if(input.action_1) {
            for(int i = 0; i < count_Ground; i++) {
                Entity* e = pool_Ground[i].entity;
                e->orientation = rotation(cubes_rotation, cubes_rotation);
                e->scale = abs(sin(my_time.since_start));
            }
        }
    }

    // Calculate movement
    for(int i = 0; i < count_Player; i++) {
        auto player = &pool_Player[i];
        auto move = i == 0 ? input.move : Vec3{-1,0,0};
        auto my_grounded_speed = grounded_speed;

        if(player->entity->position.y < -50) {
            player->velocity = {};
            reset_scene();
        }

        if(input.action_2 && do_the_thing_cooldown > 2.0f) {
            create_ground(player->entity->position, player->dimensions.x, player->dimensions.y, player->dimensions.z);
            do_the_thing_cooldown = 0;
        }
        else {
            do_the_thing_cooldown += dt;
        }
        
        // Update impulses before integration, since the input we are currently processing actually happened last frame.

        //if(character_selected) {
            if(move.y == 1 && player->grounded) {
                player->velocity.y += 15;
                player->grounded = false;
            }

            if(player->grounded) {
                player->velocity.x = 0;
                player->velocity.z = 0;
            }

            auto normalized_move = normalize(Vec3{move.x, 0, -move.z});
            if(player->grounded) {
                player->velocity.x = my_grounded_speed * normalized_move.x;
                player->velocity.z = my_grounded_speed * normalized_move.z;
            } else {
                player->velocity.x += 5 * normalized_move.x * dt;
                player->velocity.z += 5 * normalized_move.z * dt;
            }

            // Velocity integration before position integration ("Semi-implicit" Euler)
            player->velocity += g_acceleration * dt;
            
            // Position integration
            player->entity->position += player->velocity * dt;
        //}
    }

    for(int i = 0; i < count_Player; i++) {
        pool_Player[i].grounded = false;
    }

    void solve_collisions2();
    solve_collisions2();
}

void solve_collisions1() {
    for(int k = 0; k < 1; k++) {
        //for(int i = count_Player - 1; i >= 0; i--) {
        for(int i = 0; i < count_Player; i++) {
            auto player = &pool_Player[i];
            auto mat = (MaterialBasic*)player->entity->material;

            for(int j = entity_count - 1; j >= 0; j--) {
                auto other_entity = &entities[j];
                if(other_entity == player->entity) continue;
                //if(other_entity == pool_Player[])
                if(other_entity->collider.shape == ColliderShape::NONE) continue;

                AABB player_aabb = get_transformed_collider(player->entity);
                AABB other_aabb  = get_transformed_collider(other_entity);
                CollisionContact contact;
                bool overlap = collide_aabb_aabb(&player_aabb, &other_aabb, &contact);

                if(overlap) {
                    auto was_grounded = player->grounded;
                    player->entity->position += Vec3{contact.normal * (contact.penetration + 0.0001f)};
                    if(contact.normal.y == 1) {
                        player->grounded = true;
                    }

                    if(other_entity->type == EntityType::Player) {
                        auto other_player = (Player*)other_entity->type_specific_data;
                        if(contact.normal.y == 1) {
                            auto relative_velocity = vec2(other_player->velocity - player->velocity);
                            auto relative_speed_along_normal = dot(relative_velocity, contact.normal);
                            if(relative_speed_along_normal > 0) {
                                player->velocity += Vec3{contact.normal * relative_speed_along_normal};
                            }
                        } else {
                            player->velocity -= Vec3{contact.normal * dot(vec2(player->velocity), contact.normal)};
                        }
                    }
                    else {
                        player->velocity -= Vec3{contact.normal * dot(vec2(player->velocity), contact.normal)};
                    }
                    printf("Overlapped: player %d. grounded: %d normal: (%f,%f). penetration: %f \n", i, player->grounded, contact.normal.x, contact.normal.y, contact.penetration);
                }
            }
        }
    }
}

CollisionContact static_contacts[64];
int static_contacts_count = 0;
CollisionContact dynamic_contacts[64];
int dynamic_contacts_count = 0;
void solve_collisions2() {
    for(int k = 0; k < 4; k++) {
        static_contacts_count = 0;
        dynamic_contacts_count = 0;
        //for(int i = count_Player - 1; i >= 0; i--) {
        for(int i = 0; i < entity_count; i++) {
            if(entities[i].type != EntityType::Player) continue;
            
            Player* player = nullptr;
            for(int s = 0; s < count_Player; s++) {
                if(pool_Player[s].entity == &entities[i]) {
                    player = &pool_Player[s];
                    break;
                }
            }

            for(int j = entity_count - 1; j > i; j--) {
                auto other_entity = &entities[j];
                if(other_entity == player->entity) continue;
                if(other_entity->collider.shape == ColliderShape::NONE) continue;

                AABB player_aabb = get_transformed_collider(player->entity);
                AABB other_aabb  = get_transformed_collider(other_entity);
                CollisionContact* contacts = other_entity->type == EntityType::Player ? dynamic_contacts : static_contacts;
                int* contacts_count = other_entity->type == EntityType::Player ? &dynamic_contacts_count : &static_contacts_count;
                bool overlap = collide_aabb_aabb(&player_aabb, &other_aabb, &contacts[*contacts_count]);
                if(overlap) {
                    contacts[*contacts_count].player = player;
                    contacts[*contacts_count].other_entity = other_entity;
                    (*contacts_count)++;
                }
            }
        }

        for(int i = 0; i < dynamic_contacts_count; i++) {
            auto& contact = dynamic_contacts[i];
            auto& player  = contact.player;
            auto& other_entity = contact.other_entity;
            
            player->entity->position += Vec3{contact.normal * (contact.penetration / 2 + 0.0001f)};
            if(contact.normal.y == 1) {
                player->grounded = true;
            }

            other_entity->position -= Vec3{contact.normal * (contact.penetration / 2 + 0.0001f)};
            auto other_player = (Player*)other_entity->type_specific_data;
            auto relative_velocity = vec2(other_player->velocity - player->velocity);
            
            if(contact.normal.y == 1) {
                auto relative_speed_along_normal = dot(relative_velocity, contact.normal);
                if(relative_speed_along_normal > 0) {
                    player->velocity += Vec3{contact.normal * relative_speed_along_normal};
                }
            } else if(contact.normal.y == -1) {
                other_player->grounded = true;
                auto relative_speed_along_normal = dot(relative_velocity, contact.normal);
                if(relative_speed_along_normal > 0) {
                    other_player->velocity -= Vec3{contact.normal * relative_speed_along_normal};
                }
            }
            else {
                player->velocity -= Vec3{contact.normal * dot(vec2(player->velocity), contact.normal)};
                other_player->velocity += Vec3{contact.normal * dot(vec2(other_player->velocity), contact.normal)};
            }
            //printf("Overlapped: contact %d. e1: %p. e2: %p. grounded: %d normal: (%f,%f). penetration: %f \n", i, player->entity, other_entity, player->grounded, contact.normal.x, contact.normal.y, contact.penetration);
        }

        for(int i = 0; i < static_contacts_count; i++) {
            auto& contact = static_contacts[i];
            auto& player  = contact.player;

            player->entity->position += Vec3{contact.normal * (contact.penetration / 2 + 0.0001f)};
            if(contact.normal.y == 1) {
                player->grounded = true;
            }

            player->velocity -= Vec3{contact.normal * dot(vec2(player->velocity), contact.normal)};
            //printf("Overlapped: contact %d. e1: %p. e2: STATIC. grounded: %d normal: (%f,%f). penetration: %f \n", i, player->entity, player->grounded, contact.normal.x, contact.normal.y, contact.penetration);
        }
    }
}
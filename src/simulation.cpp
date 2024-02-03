#include "entities.h"
//#include "ui.h"

/*
    @Journey 8/29/2023: Ok, computational physics is hard! The math part isn't that bad, but the algorithmic part...
    I just wanted to have a few characters that collided with each other and even that lead me through a long rabbit hole
    filled with many different choices (of which the majority are compromises between accuracy and performance) and a bunch
    of math I never thought I would need. I knew the basics of rigidbody physics and how its implemented in most games
    but I wanted to know if there was a way to do things that were a little bit more robust and tailored to my specific game.

    I don't even know where to begin summarizing it so I'll just write down some of the key ideas I found:

    Basically every rule you want your objects to follow in a rigidbody simulation can be modeled as a constraint.
    There's constraints at different levels: Forces, Impulses and Positions.
    
    Force constraints are almost never used because they make for unstable simulations. For example, an "instantaneous"
    collision between two bodies changes their velocity instantaneously which would mean the forces associated would be
    infinite, so any calculations that depended on those forces would break at that point. There are more reasons why
    working with forces is normally avoided, but they all have to do with stability.

    Velocity and position constraints are what most engines use to model contacts (aka. collisions) and joints. Solving velocity constraints is hard
    if you have actual bodies that have gravity and rotate and also if you have many different kinds of joints. The difficulty lies in
    the fact that to be accurate, all constraints must be satisfied at the same time, and what makes it worse, constraints tend to
    contradict each other. For example, if you have a body pushing another against a wall, the wall pushes to one side and the pushing body to the other.
    Even such a simple case makes the problem too hard solve analitically, even more so for the general case, and so most constraint solvers work iteratively
    When you formulate the problem as an actual math problem you almost always end up with known problem called Linear Complementary Problem (LCP) or Mixed Linear
    Complementary Problem (MLCP), depending on how many and what types of constraints you want to have.

    There are many (shitty) ways to solve, or approximate I should say, a solution to these kinds of problems. There's numerical methods involving matrix math, but there's
    also the way that Box2D does it, which I like because it's equivalent to the other methods but it allows you to write code that actually has a meaning when
    read, as opposed to a bunch of matrix math with probably generically named variables that's hard to read and understand. So I took a look at most of the code base of
    Box2D and learned a lot, but it also gave me enough perspective to now throw away what I don't need and even try some new stuff.

    As always after understanding the whole problem, I'm going to simplify and start with the bare minimum that I think I need, make sure it works and then build
    from that. For now what I'm doing is:

    - Update veolcity and integrate the positions for each character. This is treated as a tentative position, since we don't know if we are going to collide with
        anything.

    - Check collisions and gather all contacts for characters penetrating each other or the environment.

    - Solve contacts as position constraints (I'm not using velocity contrainst, since I don't have "real" physics yet. I may add a velocity constraint solving phase later).
        I see a lot of engines and tutorials (I believe even Box2D) doing this step at the beginning of the frame. I really don't understand why they do that, since solving at
        the beginning and then integrating the positions can potentially leave bodies penetrating before the render phase, so the player might see them. Maybe there's a reason
        they do that, but since I can't see any for now I'm trying to solve them as close to the end as possible. The solver is an iterative one, and it's equivalent to the
        non linear Gauss Seidel algorithm, because we iterate multiple times over the constraints (contacts) resolving them while minimizing the distance between the current
        position and the tentative position of the bodies one at a time, hoping that the system eventually converges.

    - While solving position constraints, since bodies move, new collisions might happen, so every time I move a body I check if that generated any new collisions and add them
        to the contacts array. This might be too slow in the end (or maybe not) but it seems to make the simulation a lot more stable when there are multiple bodies involved.
        I can always go back and optimize or remove this.

    - Iterate one more time over the contacts, now adjusting velocities and doing any game logic that depends on them.

    EDIT 9/5/2023: I found out that apparently what I did is almost identical to something called Position Based Dynamics (PBD) and its one of the state of the art ways of
    doing simulations (there's a bunch of papers by the NVIDIA folks on the subject). It was developed for cloth and soft bodies, but has since been adapted to all kinds
    of simulations, including rigid bodies. So I guess I'm heading in the right direction, lets keep developing this idea.
*/

//All these are @TEMPORARY
float cubes_rotation_dir = 1;
float cubes_rotation = 0;
bool paused = false;
bool character_selected = true;
float camera_distance_target = 10;
float camera_distance = 10;
float zoom_sensitivity = 2.5f;
float zoom_snappiness = 10.0f;
Vec2i saved_mouse_pos = {0,0};

// Player movement
const Vec3 g_acceleration = {0, 3 * -9.8f, 0};
float grounded_speed = 10;
float jump_speed = 12;

static void update_camera(float dt) {
    float delta_yaw = -input.mouse_raw_motion.x * radians(2.0f) * dt;
    main_camera->yaw = normalize_angle(main_camera->yaw + delta_yaw);
    
    float delta_pitch = input.mouse_raw_motion.y * radians(2.0f) * dt;
    main_camera->pitch = clamp(main_camera->pitch + delta_pitch, radians(-89), radians(89));

    Vec3 forward = angles_to_vec(main_camera->yaw, main_camera->pitch);

    if(using_perspective) {
        if(character_selected) {
            main_camera->target.x = exp_interpolate(main_camera->target.x, main_player->entity->position.x, dt);
            main_camera->target.y = exp_interpolate(main_camera->target.y, main_player->entity->position.y, dt);
            main_camera->target.z = main_player->entity->position.z;
            static auto last_player_pos = 0.0f;
            auto player_dx = (main_player->entity->position.x - last_player_pos) / dt;
            last_player_pos = main_player->entity->position.x;
            camera_distance_target = fabs(player_dx) * 8;
            camera_distance_target = clamp(camera_distance_target, 10.0f, 25.0f);
            camera_distance = exp_interpolate(camera_distance, camera_distance_target, dt, 0.1f);
            main_camera->entity->position = main_camera->target + normalize(Vec3{0, 4, 7}) * camera_distance;
        }
        else {
            if(input.move.x != 0) {
                main_camera->entity->position += normalize(cross(Vec3{0,1,0}, -forward)) * 5.0f * input.move.x * dt;
            }

            if(input.move.y != 0) {
                main_camera->entity->position += Vec3{0,1,0} * 5.0f * input.move.y * dt;
            }

            if(input.move.z != 0) {
                main_camera->entity->position += forward * 5.0f * input.move.z * dt;
            }
        }
    }
    else {
        forward = Vec3::forward;
        if(character_selected) {
            main_camera->target.x = exp_interpolate(main_camera->target.x, main_player->entity->position.x, dt);
            main_camera->target.y = exp_interpolate(main_camera->target.y, main_player->entity->position.y, dt);
            main_camera->entity->position = main_camera->target + Vec3::back * camera_distance;
        }
        else {
            if(is_key_down(VK_RBUTTON)) {
                Vec2i new_mouse_pos = input.mouse_pos_pixels;
                if(input.mouse_pos_pixels.x < 5)                       new_mouse_pos.x = window->size.x - 5;
                else if(input.mouse_pos_pixels.x > window->size.x - 5) new_mouse_pos.x = 5;
                
                if(input.mouse_pos_pixels.y < 5)                       new_mouse_pos.y = window->size.y - 5;
                else if(input.mouse_pos_pixels.y > window->size.y - 5) new_mouse_pos.y = 5;

                if(new_mouse_pos != input.mouse_pos_pixels) set_mouse_pos(new_mouse_pos.x, new_mouse_pos.y);
                
                //@Cleanup: Store the ratio of world units to pixels somewhere.
                float pixels_per_world_unit = ORTHOGRAPHIC_VIEW_WIDTH / window->size.x;
                main_camera->entity->position.x -= input.mouse_delta_pixels.x * pixels_per_world_unit;
                main_camera->entity->position.y += input.mouse_delta_pixels.y * pixels_per_world_unit;
                main_camera->target = main_camera->entity->position;
            }
        }
    }

    per_frame_uniforms.view_pos = main_camera->entity->position;
    per_frame_uniforms.view = look_to(main_camera->entity->position, forward);
}

float do_the_thing_cooldown = 0;
void simulate(float dt) {
    {
        auto r = is_button_down(INPUT_BUTTON_ROTATE_RIGHT) - is_button_down(INPUT_BUTTON_ROTATE_LEFT);
        cubes_rotation += -r * radians(45.0f) * dt;
        
        if(is_button_down(INPUT_BUTTON_WEIRD)) {
            for(int i = 0; i < count_Ground; i++) {
                Entity* e = pool_Ground[i].entity;
                e->orientation = rotation(cubes_rotation, cubes_rotation);
                e->scale = fabs(sinf(my_time.since_start)) * Vec3{1,1,1};
            }
        }
    }

    if(is_button_down(INPUT_BUTTON_DO_THE_THING) && do_the_thing_cooldown >= 1.0f) {
        create_ground(main_player->entity->position, main_player->entity->scale.x, main_player->entity->scale.y, main_player->entity->scale.z);
        do_the_thing_cooldown = 0;
    }
    else {
        do_the_thing_cooldown = min(do_the_thing_cooldown + dt, 1.0f);
    }

    // Calculate movement
    for(int i = 0; i < count_Player; i++) {
        auto player = &pool_Player[i];

        if(player->entity->position.y < -50) {
            player->velocity = {};
            reset_scene();
            return;
        }
        
        // Update impulses before integration, since the input we are currently processing actually happened last frame.

        if(i == 0) {
            if(!paused && using_perspective && !character_selected) continue;
            player->move = input.move;
        }
        else {
            player->move = Vec3{(float)(is_key_down('L') - is_key_down('J')), (float)is_key_down('I'), 0};
        }

        player->jumped_this_frame = false;
        auto target_velocity = player->move * 30;
        if(player->grounded) {
            if(player->move.y == 1) {
                if(player->velocity.y < jump_speed) player->velocity.y += jump_speed;
                player->jumped_this_frame = true;
            }
            else {
                auto acc = 10.0f;
                if(target_velocity.x == 0 || sign(target_velocity.x) != sign(player->velocity.x)) acc *= 4;

                if(target_velocity.x > player->velocity.x) {
                    player->velocity.x += acc * dt;
                    if(player->velocity.x > target_velocity.x) player->velocity.x = target_velocity.x;
                }
                else if(target_velocity.x < player->velocity.x) {
                    player->velocity.x -= acc * dt;
                    if(player->velocity.x < target_velocity.x) player->velocity.x = target_velocity.x;
                }
                player->velocity.z = grounded_speed * player->move.z;
            }
        }
        else {
            player->velocity.x += 5 * player->move.x * dt;
            player->velocity.z += 5 * player->move.z * dt;
        }

        // Velocity integration before position integration ("Semi-implicit" Euler)
        player->velocity += g_acceleration * dt;
        
        // Position integration
        player->entity->position += player->velocity * dt;

        pool_Player[i].was_grounded = pool_Player[i].grounded;
        pool_Player[i].grounded = false;
    }

    void solve_collisions(float dt);
    solve_collisions(dt);
}

CollisionContact static_contacts[64];
int static_contacts_count = 0;
CollisionContact dynamic_contacts[64];
int dynamic_contacts_count = 0;

void find_new_contacts(Player* player) {
    AABB player_aabb = get_transformed_collider(player->entity);
    
    for(int e = 0; e < entity_count; e++) {
        if(player->entity == &entities[e]) continue;
        if(entities[e].collider.shape == COLLIDER_SHAPE_NONE) continue;

        AABB other_aabb = get_transformed_collider(&entities[e]);
        CollisionContact maybe_new_contact;
        bool overlap = collide_aabb_aabb(&player_aabb, &other_aabb, &maybe_new_contact);
        if(overlap) {
            bool is_new_contact = true;
            if(entities[e].type == ENTITY_TYPE_Player) {
                auto other_p = down_cast<Player>(&entities[e]);
                for(int j = 0; j < dynamic_contacts_count; j++) {
                    if(dynamic_contacts[j].player == player && dynamic_contacts[j].other_entity == &entities[e]) {
                        is_new_contact = false;
                        break;
                    }

                    if(dynamic_contacts[j].player == other_p && dynamic_contacts[j].other_entity == player->entity) {
                        is_new_contact = false;
                        break;
                    }
                }

                if(is_new_contact) {
                    maybe_new_contact.player = player;
                    maybe_new_contact.other_entity = &entities[e];
                    dynamic_contacts[dynamic_contacts_count++] = maybe_new_contact;
                }
            }
            else {
                for(int j = 0; j < static_contacts_count; j++) {
                    if(static_contacts[j].player == player && static_contacts[j].other_entity == &entities[e]) {
                        is_new_contact = false;
                        break;
                    }
                }

                if(is_new_contact) {
                    maybe_new_contact.player = player;
                    maybe_new_contact.other_entity = &entities[e];
                    static_contacts[static_contacts_count++] = maybe_new_contact;
                }
            }
        }
    }
}

bool maybe_resolve_air_collision(bool a_grounded, bool b_grounded, Vec3 relative_velocity_along_normal, float restitution, Vec3* a_velocity, Vec3* b_velocity) {
    if(a_grounded) {
        if(b_grounded) {
            return false;
        }
        else {
            *b_velocity += relative_velocity_along_normal * restitution;
        }
    }
    else if(b_grounded) {
        *a_velocity -= relative_velocity_along_normal * restitution;
    }
    else {
        auto impulse = relative_velocity_along_normal * (1 + restitution) / 2;
        *a_velocity -= impulse;
        *b_velocity += impulse;
    }

    return true;
}

void solve_collisions(float dt) {
    static_contacts_count = 0;
    dynamic_contacts_count = 0;
    for(int i = 0; i < entity_count; i++) {
        if(entities[i].type != ENTITY_TYPE_Player) continue;

        Player* player = down_cast<Player>(&entities[i]);

        for(int j = i + 1; j < entity_count; j++) {
            auto other_entity = &entities[j];
            if(other_entity == player->entity) continue;
            if(other_entity->collider.shape == COLLIDER_SHAPE_NONE) continue;

            AABB player_aabb = get_transformed_collider(player->entity);
            AABB other_aabb  = get_transformed_collider(other_entity);
            
            CollisionContact* contacts = other_entity->type == ENTITY_TYPE_Player ? dynamic_contacts : static_contacts;
            int* contacts_count = other_entity->type == ENTITY_TYPE_Player ? &dynamic_contacts_count : &static_contacts_count;
            
            bool overlap = collide_aabb_aabb(&player_aabb, &other_aabb, &contacts[*contacts_count]);
            if(overlap) {
                contacts[*contacts_count].player = player;
                contacts[*contacts_count].other_entity = other_entity;
                (*contacts_count)++;
            }
        }
    }

#if DEBUG_PHYSICS
    printf("===START===\n");
    printf("%d, %d\n", static_contacts_count, dynamic_contacts_count);
    printf("%f\n", pool_Player[0].entity->position.x);
    printf("%f\n", pool_Player[1].entity->position.x);
    printf("%f\n", pool_Player[2].entity->position.x);
    printf("===\n");
#endif

    for(int k = 0; k < 10; k++) {
        bool early_out = true;

        for(int i = 0; i < dynamic_contacts_count; i++) {
            auto& contact     = dynamic_contacts[i];
            auto player       = contact.player;
            auto other_entity = contact.other_entity;
            
            AABB player_aabb = get_transformed_collider(player->entity);
            AABB other_aabb  = get_transformed_collider(other_entity);

            // @Speed: There's no need to do a full collide here, we only want to update the separation
            // between the bodies involved in a contact we already have.
            bool overlap = collide_aabb_aabb(&player_aabb, &other_aabb, &contact);
            if(overlap) {
                early_out = false;
                //? Right now we allow characters pushing each other based on how much interpenetration happend
                //? Maybe we can use relative velocity to resolve the penetration according to who moved the most?
                player->entity->position += Vec3{contact.normal * (contact.penetration / 2)};
                other_entity->position   -= Vec3{contact.normal * (contact.penetration / 2)};

                //TODO: Proper grounded checking
                auto other_player = down_cast<Player>(other_entity);
                if(contact.normal.y == 1 && !player->jumped_this_frame) {
                    player->grounded = true;
                }
                else if(contact.normal.y == -1 && !player->jumped_this_frame) {
                    other_player->grounded = true;
                }

#if DEBUG_PHYSICS
                printf("===CHANGE===\n");
                printf("%f\n", player->entity->position.x);
                printf("%f\n", other_entity->position.x);
                printf("===\n");
#endif

                find_new_contacts(player);
                find_new_contacts(other_player);
            }
        }

        for(int i = 0; i < static_contacts_count; i++) {
            auto& contact = static_contacts[i];
            auto player   = contact.player;

            AABB player_aabb = get_transformed_collider(player->entity);
            AABB other_aabb  = get_transformed_collider(contact.other_entity);
            
            // @Speed: There's no need to do a full collide here, we only want to update the separation
            // between the bodies involved in a contact we already have.
            bool overlap = collide_aabb_aabb(&player_aabb, &other_aabb, &contact);
            if(overlap) {
                early_out = false;
                player->entity->position += Vec3{contact.normal * contact.penetration};

                //TODO: Proper grounded checking
                if(contact.normal.y == 1 && !player->jumped_this_frame) {
                    player->grounded = true;
                }

#if DEBUG_PHYSICS
                printf("===CHANGE===\n");
                printf("%f\n", player->entity->position.x);
                printf("===\n");
#endif

                find_new_contacts(player);
            }
        }

        if(early_out) break;
    }

#if DEBUG_PHYSICS
    printf("===END===\n");
    printf("%d, %d\n", static_contacts_count, dynamic_contacts_count);
    printf("%f\n", pool_Player[0].entity->position.x);
    printf("%f\n", pool_Player[1].entity->position.x);
    printf("%f\n", pool_Player[2].entity->position.x);
    printf("===\n");
#endif

    //? How can we ensure this converges?
    for(int k = 0; k < 10; k++) {
        for(int i = 0; i < dynamic_contacts_count; i++) {
            auto& contact = dynamic_contacts[i];
            auto player   = contact.player;
            auto other_player = down_cast<Player>(contact.other_entity);
            Vec3 normal = Vec3{contact.normal};
            
            auto player_speed_along_normal = dot(player->velocity,       normal);
            auto other_speed_along_normal  = dot(other_player->velocity, normal);
            auto relative_speed_along_normal = player_speed_along_normal - other_speed_along_normal;
            
            // Going away from each other.
            if(relative_speed_along_normal >= 0) continue;
            
            auto relative_velocity_along_normal = relative_speed_along_normal * normal;

            if(player_speed_along_normal < 0) {
                if(other_speed_along_normal > 0) {
                    // Going opposite directions
                    if(normal.y == -1) {
                        other_player->velocity += relative_velocity_along_normal;
                    }
                    else if(normal.y == 1) {
                        player->velocity -= relative_velocity_along_normal;
                    }
                    else {
                        bool resolved = maybe_resolve_air_collision(player->grounded, other_player->grounded, relative_velocity_along_normal, 0.5f, &player->velocity, &other_player->velocity);
                        if(!resolved) {
                            player->velocity       -= player_speed_along_normal * normal;
                            other_player->velocity -= other_speed_along_normal  * normal;
                        }
                    }
                }
                else {
                    // Going both opposite to normal
                    bool resolved = maybe_resolve_air_collision(player->grounded, other_player->grounded, relative_velocity_along_normal, 0.5f, &player->velocity, &other_player->velocity);
                    if(!resolved) {
                        player->velocity -= relative_velocity_along_normal;
                    }
                }
            }
            else if(other_speed_along_normal > 0) {
                // Going both in direction of normal
                bool resolved = maybe_resolve_air_collision(player->grounded, other_player->grounded, relative_velocity_along_normal, 0.5f, &player->velocity, &other_player->velocity);
                if(!resolved) {
                    other_player->velocity += relative_velocity_along_normal;
                }
            }
        }

        for(int i = 0; i < static_contacts_count; i++) {
            auto& contact = static_contacts[i];
            auto player   = contact.player;

            float speed_along_normal = dot(vec2(player->velocity), contact.normal);
            if(speed_along_normal < 0) player->velocity -= Vec3{contact.normal * speed_along_normal};
        }
    }
}
#include "entities.h"
#include "input.h"
#include "general.h"
#include "render.h"
#include "simulation.h"
#include "editor.h"
#include <cstdio>
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

    Velocity and position constraints are what most engines use to model contacts (aka. collisions) and joints. Solving velocity constraints is hard (computationally)
    if you have actual bodies that have gravity and rotate and also if you have many different kinds of joints. The difficulty lies in
    the fact that to be accurate, all constraints must be satisfied at the same time, and what makes it worse, constraints tend to
    contradict each other. For example, if you have a body pushing another against a wall, the wall pushes to one side and the pushing body to the other.
    Even such a simple case makes the problem too hard solve analitically, even more so for the general case, and so most constraint solvers work iteratively.
    When you formulate the problem as an actual math problem you end up with a known problem called Linear Complementary Problem (LCP) or Mixed Linear
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
bool paused = false;
float zoom_sensitivity = 2.5f;
float zoom_snappiness = 10.0f;

// Player movement
Vector3 move;
const Vector3 g_acceleration = {0, 3 * -9.8f, 0};
float grounded_speed = 10;
float jump_speed = 12;

void process_game_input(OsEvent* event) {
    move.x = (float)(is_key_down('D') - is_key_down('A'));
    move.y = (float)is_key_down(VK_SPACE);
    move.z = -(float)(is_key_down('W') - is_key_down('S'));   // Flipped because -Z is forward
    
    for(auto e = os_events; e != nullptr; e = e->next) {
        auto key = event->key; 
        switch(key.keycode) {
            case VK_LBUTTON: {
                if(key.state & JUST_PRESSED) {                
                    auto c = get_entity(main_camera);
                    if(c) {
                        Entity* closest = nullptr;
                        float closest_t = MAX_FLOAT;
                        ForP(manager->entities.all) {
                            if((it->flags & (ENTITY_FLAG_ACTIVE | ENTITY_FLAG_MOUSE_PICKABLE)) != (ENTITY_FLAG_ACTIVE | ENTITY_FLAG_MOUSE_PICKABLE)) continue;

                            auto t = raycast(c->position, c->camera->forward, get_world_space_collider(it));
                            if(t > 0.0001f && t < closest_t) {
                                closest = it;
                                closest_t = t;
                            }
                        }
                        if(closest) {
                            editor_selected = closest->handle;
                        } else {
                            editor_selected = INVALID_ENTITY_HANDLE;
                        }
                    }
                }
                break;
            }

            case VK_LEFT: {
                if(key.state & JUST_PRESSED) my_time.sim_scale *= 0.5f;
                break;
            }

            case VK_RIGHT: {
                if(paused && key.state & IS_DOWN) {
                    do_step = true;
                }
                else if(key.state & JUST_PRESSED) {
                    my_time.sim_scale *= 2.0f;
                }
                break;
            }

            case 'P': {
                if(key.state & JUST_PRESSED) paused = !paused;
                break;
            }

            case 'R': {
                if(key.state & JUST_PRESSED) reset_scene();
                break;
            }

            case 'T': {
                if(key.state & JUST_PRESSED) using_perspective = !using_perspective;
                break;
            }

            case VK_TAB: {
                if(key.state & JUST_PRESSED) {
                    editor_on = !editor_on;
                    capture_and_hide_mouse(event->window, !editor_on);
                }
                break;
            }

            case VK_F11: {
                if(key.state & JUST_PRESSED) {
                    bool fullscreen = is_fullscreen(event->window);
                    set_fullscreen(event->window, !fullscreen);
                }
                break;
            }
        }
    }
}

void update_camera(float dt) {
    auto camera_e = get_entity(main_camera);
    auto camera = camera_e->camera;

    const float mouse_sensitivity = 3.5f;

    float delta_yaw = -mouse_delta_raw.x * radians(2.0f) * dt * mouse_sensitivity;
    camera->yaw = normalize_angle(camera->yaw + delta_yaw);
    
    float delta_pitch = mouse_delta_raw.y * radians(2.0f) * dt * mouse_sensitivity;
    camera->pitch = clamp(camera->pitch + delta_pitch, radians(-89), radians(89));

    auto selected = get_entity(editor_selected);
    if(using_perspective) {
        camera->forward = angles_to_vector(camera->yaw, camera->pitch);

        const auto camera_dir = normalize(Vector3{0, 4, 7});
        
        if(editor_on || !selected) {
            if(move.x != 0) {
                camera_e->position += normalize(cross(Vector3{0,1,0}, -camera->forward)) * 5.0f * move.x * dt;
            }

            if(move.y != 0) {
                camera_e->position += Vector3{0,1,0} * 5.0f * move.y * dt;
            }

            if(move.z != 0) {
                camera_e->position -= camera->forward * 5.0f * move.z * dt;
            }

            camera->distance = camera_e->position.z / dot(Vector3{0,0,1}, camera_dir);
            camera->target = (Vector2)(camera_e->position - camera_dir * camera->distance);
        }
        else if(selected) {
            camera->target.x = exp_interpolate(camera->target.x, selected->position.x, dt);
            camera->target.y = exp_interpolate(camera->target.y, selected->position.y, dt);

            if(selected->type == ENTITY_TYPE_Player) {
                auto player = selected->player;
                auto target_distance = fabsf(player->velocity.x) * 8;
                target_distance = clamp(target_distance, 10.0f, 25.0f);
                float snappiness = 0.1f;
                if(camera->distance < 10) {
                    snappiness = lerp(0.1f, 10, 1 - camera->distance / 10);
                } else if(camera->distance > 25) {
                    snappiness = lerp(0.1f, 5, 1 - 25 / camera->distance);
                }
                camera->distance = exp_interpolate(camera->distance, target_distance, dt, snappiness);
            }

            camera_e->position = (Vector3)camera->target + camera_dir * camera->distance;
        }
    }
    else {
        camera->forward = Vector3::forward;
        if(editor_on) {
            if(keystates[VK_RBUTTON] & IS_DOWN) {
                Vector2i new_mouse_pos = mouse_position;
                Vector2i window_size = get_window_size(the_window);

                if(mouse_position.x < 5) new_mouse_pos.x = window_size.x - 5;
                else if(mouse_position.x > window_size.x - 5) new_mouse_pos.x = 5;
                
                if(mouse_position.y < 5) new_mouse_pos.y = window_size.y - 5;
                else if(mouse_position.y > window_size.y - 5) new_mouse_pos.y = 5;

                if(new_mouse_pos != mouse_position) set_mouse_position(new_mouse_pos.x, new_mouse_pos.y);
                
                //@Cleanup: Store the ratio of world units to pixels somewhere.
                float pixels_per_world_unit = ORTHOGRAPHIC_VIEW_WIDTH / window_size.x;
                camera_e->position.x -= mouse_delta_pixels.x * pixels_per_world_unit;
                camera_e->position.y += mouse_delta_pixels.y * pixels_per_world_unit;
            }
        }
        else if(selected) {
            camera->target = selected->position.xy;
            camera_e->position = camera->target + Vector3::back * 5;
        }
    }
}

void simulate(float dt) {
    auto selected = get_entity(editor_selected);

    // Calculate movement
    for(int i = 0; i < manager->players.all.count; i++) {
        auto it = &manager->players.all[i];

        if(it->entity->position.y < -50) {
            reset_scene();
            return;
        }

        if(it->entity == selected) {
            if(editor_on) {
                it->move = {};
            } else {
                it->move = move;
            }
        }
        else {
            it->move = Vector3{(float)((bool)(keystates['L'] & IS_DOWN) - (bool)(keystates['J'] & IS_DOWN)), (float)(bool)(keystates['I'] & IS_DOWN), 0};
        }

        it->jumped_this_frame = false;
        auto target_velocity = it->move * 30;
        if(it->grounded) {
            if(it->move.y == 1) {
                if(it->velocity.y < jump_speed) it->velocity.y += jump_speed;
                it->jumped_this_frame = true;
            }
            else {
                auto acc = 10.0f;
                if(target_velocity.x == 0 || sign(target_velocity.x) != sign(it->velocity.x)) acc *= 4;

                if(target_velocity.x > it->velocity.x) {
                    it->velocity.x += acc * dt;
                    if(it->velocity.x > target_velocity.x) it->velocity.x = target_velocity.x;
                }
                else if(target_velocity.x < it->velocity.x) {
                    it->velocity.x -= acc * dt;
                    if(it->velocity.x < target_velocity.x) it->velocity.x = target_velocity.x;
                }
                it->velocity.z = grounded_speed * it->move.z;
            }
        }
        else {
            it->velocity.x += 5 * it->move.x * dt;
            it->velocity.z += 5 * it->move.z * dt;
        }

        // Velocity integration before position integration ("Semi-implicit" Euler)
        it->velocity += g_acceleration * dt;
        
        // Position integration
        it->entity->position += it->velocity * dt;

        it->was_grounded = it->grounded;
        it->grounded = false;
    }

    void solve_collisions(float dt);
    solve_collisions(dt);
}

CollisionContact static_contacts[64];
int static_contacts_count = 0;
CollisionContact dynamic_contacts[64];
int dynamic_contacts_count = 0;

void find_new_contacts(Player* player) {
    AABB player_aabb = get_world_space_collider(player->entity);
    
    ForP(manager->entities.all) {
        if(!(it->flags & ENTITY_FLAG_ACTIVE) || player->entity == it) continue;
        if(it->collider.shape == COLLIDER_SHAPE_NONE) continue;

        AABB other_aabb = get_world_space_collider(it);
        CollisionContact maybe_new_contact;
        bool overlap = collide_aabb_aabb(&player_aabb, &other_aabb, &maybe_new_contact);
        if(overlap) {
            bool is_new_contact = true;
            if(it->type == ENTITY_TYPE_Player) {
                auto other_p = it->player;
                for(int j = 0; j < dynamic_contacts_count; j++) {
                    if(dynamic_contacts[j].player == player && dynamic_contacts[j].other_entity == it) {
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
                    maybe_new_contact.other_entity = it;
                    dynamic_contacts[dynamic_contacts_count++] = maybe_new_contact;
                }
            }
            else {
                for(int j = 0; j < static_contacts_count; j++) {
                    if(static_contacts[j].player == player && static_contacts[j].other_entity == it) {
                        is_new_contact = false;
                        break;
                    }
                }

                if(is_new_contact) {
                    maybe_new_contact.player = player;
                    maybe_new_contact.other_entity = it;
                    static_contacts[static_contacts_count++] = maybe_new_contact;
                }
            }
        }
    }
}

bool maybe_resolve_air_collision(bool a_grounded, bool b_grounded, Vector3 relative_velocity_along_normal, float restitution, Vector3* a_velocity, Vector3* b_velocity) {
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
    for(int i = 0; i < manager->entities.all.count; i++) {
        auto e = &manager->entities.all[i];
        if(!(e->flags & ENTITY_FLAG_ACTIVE) || e->type != ENTITY_TYPE_Player) continue;

        Player* player = e->player;

        for(int j = i + 1; j < manager->entities.all.count; j++) {
            auto other_entity = &manager->entities.all[j];
            if(!(other_entity->flags & ENTITY_FLAG_ACTIVE) || other_entity == e) continue;
            if(other_entity->collider.shape == COLLIDER_SHAPE_NONE) continue;

            AABB player_aabb = get_world_space_collider(e);
            AABB other_aabb  = get_world_space_collider(other_entity);
            
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
            
            AABB player_aabb = get_world_space_collider(player->entity);
            AABB other_aabb  = get_world_space_collider(other_entity);

            // @Speed: There's no need to do a full collide here, we only want to update the separation
            // between the bodies involved in a contact we already have.
            bool overlap = collide_aabb_aabb(&player_aabb, &other_aabb, &contact);
            if(overlap) {
                early_out = false;
                //? Right now we allow characters pushing each other based on how much interpenetration happend
                //? Maybe we can use relative velocity to resolve the penetration according to who moved the most?
                player->entity->position += contact.normal * (contact.penetration / 2);
                other_entity->position   -= contact.normal * (contact.penetration / 2);

                //TODO: Proper grounded checking
                auto other_player = other_entity->player;
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

            AABB player_aabb = get_world_space_collider(player->entity);
            AABB other_aabb  = get_world_space_collider(contact.other_entity);
            
            // @Speed: There's no need to do a full collide here, we only want to update the separation
            // between the bodies involved in a contact we already have.
            bool overlap = collide_aabb_aabb(&player_aabb, &other_aabb, &contact);
            if(overlap) {
                early_out = false;
                player->entity->position += contact.normal * contact.penetration;

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
            auto other_player = contact.other_entity->player;
            Vector3 normal = contact.normal;
            
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

            float speed_along_normal = dot(player->velocity.xy, contact.normal);
            if(speed_along_normal < 0) player->velocity -= contact.normal * speed_along_normal;
        }
    }
}
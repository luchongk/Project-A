#include "entities.h"
#include "ui.h"

//All these are @TEMPORARY
static float cubes_rotation_dir = 1;

float cubes_rotation = 0;
bool paused = false;
bool character_selected = false;

static void update_camera(float dt) {
    float yaw = player_input.mouse_delta_pixels.x * radians(2.0f) * dt;
    camera.yaw = normalize_angle(camera.yaw + yaw);
    
    float pitch = player_input.mouse_delta_pixels.y * radians(2.0f) * dt;
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

static UIButtonState* ui_button(Rect rect, Vec4 base_color, Vec4 hover_color, int id) {
    UIButtonState* state = get_or_create(&ui_button_states, id, {rect});

    state->rect = rect; //@Temporary: We need to save this cause we don't have render commands yet.
    state->base_color  = base_color;
    state->hover_color = hover_color;
    state->clicked = false;
    bool hovering = is_mouse_over(state->rect);
    
    if(hovering && !player_input.mouse_interacting_with_ui) {
        state->hovered = true;
        
        if(player_input.left_click == KeyState::DOWN) {
            state->pressed = true;
        }
        else if(state->pressed) {
            if(player_input.left_click == KeyState::UP) {
                state->pressed = false;
                state->clicked = true;
            }
        }

        player_input.mouse_interacting_with_ui = true;
    }
    else {
        state->hovered = false;

        if(state->pressed) {
            player_input.mouse_interacting_with_ui = true;
        }

        if(player_input.left_click == KeyState::UP) {
            state->pressed = false;
        }
    }

    array_add(&ui_buttons_to_draw, state);

    return state;
}

bool should_show_blue = false;
bool follow = false;
Rect blue_rect = {0.5f, 0.5f, 0.2f, 0.1f};
Vec2 grab = {};

void simulate(float dt) {
    array_reset(&ui_buttons_to_draw);
    player_input.mouse_interacting_with_ui = false;

    Rect button_rect = {0.1f, 0.8f, 0.2f, 0.1f};
    Vec4 button_color = {0, 0, 0, 0.7f};

    auto state = ui_button(button_rect, button_color, {0.5f, 0, 0, 1.0f}, 1);
    if(state->clicked) {
        std::cout << "RED" << std::endl;
        should_show_blue = !should_show_blue;
    }
    
    button_rect.x = (sinf(time.since_start * 0.5f) + 1) / 2;
    state = ui_button(button_rect, button_color, {0, 0.5f, 0, 1.0f}, 2);
    if(state->clicked) {
        std::cout << "GREEN" << std::endl;
    }

    if(should_show_blue) {
        if(follow) {
            blue_rect.x = player_input.mouse_pos_normalized.x - grab.x;
            blue_rect.y = player_input.mouse_pos_normalized.y - grab.y;
        }
        state = ui_button(blue_rect, {0, 0, 0.5f, 1.0f}, {0, 0, 0.5f, 1.0f}, 3);
        if(state->pressed) {
            if(!follow) {
                follow = true;
                grab.x = player_input.mouse_pos_normalized.x - blue_rect.x;
                grab.y = player_input.mouse_pos_normalized.y - blue_rect.y;
            }
        }
        else {
            follow = false;
        };
        
        if(state->clicked) {
            std::cout << "BLUE" << std::endl;
        }

        state = ui_button({0,0,1,1}, {0,0,0,0.3f}, {0,0,0,0.3f}, 4);
        if(state->clicked) {
            should_show_blue = false;
        }
    }

    if(player_input.left_click == KeyState::UP) {
        std::cout << player_input.mouse_interacting_with_ui << std::endl;
    }

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

//All these are @TEMPORARY
static float cubes_rotation_dir = 1;

Camera camera;
Vector3 light_pos = {1.0f, 1.0f, -6.0f};
float cubes_rotation = 0;
bool paused = true;
float light_time_accum = 0.0f;

static void update_camera(PlayerInput* input, float dt) {
    camera.yaw += input->mouse_delta.x * 2.0f * dt;
    if(camera.yaw > 360) {
        camera.yaw -= 360;
    }
    else if(camera.yaw < 0) {
        camera.yaw += 360;
    }
    camera.pitch += -input->mouse_delta.y * 2.0f * dt;
    if(camera.pitch > 360) {
        camera.pitch -= 360;
    }
    else if(camera.pitch < 0) {
        camera.pitch += 360;
    }

    camera.forward.x = glm::cos(glm::radians(camera.yaw)) * glm::cos(glm::radians(camera.pitch));
    camera.forward.y = glm::sin(glm::radians(camera.pitch));
    camera.forward.z = glm::sin(glm::radians(camera.yaw)) * glm::cos(glm::radians(camera.pitch));
    camera.forward.normalize();

    if(input->horizontal != 0) {
        camera.position += camera.forward.cross(Vector3{0,1,0}) * 3.0f * (float)input->horizontal * dt;
    }

    if(input->vertical != 0) {
        camera.position += camera.forward * 2.0f * (float)input->vertical * dt;
    }
}

void simulate(PlayerInput* input) {
    if(paused) {
        return;
    }

    float dt = time.delta * time.modifier;

    update_camera(input, dt);
    
    light_time_accum += dt;
    cubes_rotation += 2 * glm::sin(10 * glm::radians(light_time_accum)) * cubes_rotation_dir * dt;
}
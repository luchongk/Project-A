
//All these are @TEMPORARY
static float cubes_rotation_dir = 1;

Camera camera;
Vector3 light_pos = {1.0f, 1.0f, -6.0f};
float cubes_rotation = 0;
Vector3 cubes_offset;
bool paused = false;
bool character = false;
float light_time_accum = 0.0f;

float shapeX = 0;
float shapeY = 0;

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

    if(character && input->horizontalX != 0) {
        camera.position += camera.forward.cross(Vector3{0,1,0}) * 3.0f * (float)input->horizontalX * dt;
    }

    if(character && input->horizontalZ != 0) {
        camera.position += camera.forward * 2.0f * (float)input->horizontalZ * dt;
    }

    if(character && input->vertical != 0) {
        camera.position += Vector3{0,1,0} * 2.0f * (float)input->vertical * dt;
    }
}

void simulate(PlayerInput* input) {
    if(paused) {
        return;
    }

    float dt = time.delta * time.modifier;

    update_camera(input, dt);
    
    //light_time_accum += (float)time.delta;
    if(!character) {
        //per_object_uniforms.material.diffuse += glm::vec3{0.0f, 0.0f, input->vertical * dt};
        cubes_offset.x += input->horizontalX * 3.0f * dt;
        cubes_offset.y += input->vertical * 3.0f * dt;
        cubes_offset.z -= input->horizontalZ * 3.0f * dt;
    }
    cubes_rotation -= input->rotationDir * 45.0f * dt;
    //cubes_rotation += 2 * glm::sin(10 * glm::radians(light_time_accum)) * cubes_rotation_dir * dt;
    light_pos = {
        glm::cos(light_time_accum * 0.75f) * 2,
        glm::sin(light_time_accum * 0.75f) * 2,
        -6.0f
    };
}
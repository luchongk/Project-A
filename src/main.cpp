#include "platform.h"
#include "main.h"

#ifdef _WIN32
    #include "platform_win32.cpp"
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.cpp"
#include "shader.cpp"
#include "input.cpp"
#include "render.cpp"
#include "simulation.cpp"
#include "obj_loader.cpp"

Allocator default_allocator = malloc_allocator;
void* default_allocator_data;

Time time;
float maxFrameTime = 0.25f;

static void update_time() {
    s64 now = os_get_timestamp();
    
    time.delta = (now - time.last_stamp) / (float)time.frequency;
    if(time.delta > maxFrameTime) {
        //Framerate too low, we are falling behind in the simulation! D: Preventing spiral of death.
        time.delta = maxFrameTime;
    }

    time.since_start = (now - time.start_stamp) / (float)time.frequency;
    time.last_stamp = now;
}

void main() {
    PlayerInput input{};  //Not sure where to put this, I'll leave it there for now.

    OSWindow* window = os_create_window();

    time.start_stamp = os_get_timestamp();
    time.frequency = os_get_timer_frequency();
    time.simulation_delta = 1.0f/60;
    time.modifier = 1.0f;
    maxFrameTime = 0.25f;

    float accum = 0;

    init();

    bool exit = false;
    while(!exit) {
        update_time();

        handle_events(window, &input);
        
        accum += time.delta;
        while(accum >= time.simulation_delta) {
            simulate(&input);
            accum -= time.simulation_delta;
        }

        //TODO: Stuttering comes from not doing position/rotation interpolation. Do that.
        //float lerp = accum / fixedDeltaTime;
        render(window);

#if 0
    printf_s("last frame: %f ms\n", time.delta * 1000);
    printf_s("FPS: %f\n", 1 / time.delta);
    fflush(stdout);   //CPU usage goes 10x and my laptops fan starts going crazy if we do this every frame. WTF!
#endif

        exit = os_poll_events();
    }
}
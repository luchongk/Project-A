#include "platform.h"
#include "main.h"

#ifdef _WIN32
    #include "platform_win32.cpp"
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.cpp"
#include "input.cpp"
#include "render.cpp"
#include "simulation.cpp"
#include "graphics.cpp"
#include "obj_loader.cpp"
#include "entities.cpp"

Time time;
OSWindow* window;

static void update_time() {
    float now = os_get_time();
    
    time.since_start = now - time.start_at;
    
    time.dt = now - time.last_frame;
    if(time.dt > time.max_allowed_per_frame) {
        //Framerate too low, we are falling behind in the simulation! D: Preventing spiral of death.
        time.dt = time.max_allowed_per_frame;
    }

    time.last_frame = now;
}

void main() {
    time.start_at = os_get_time();
    time.sim_dt = 1.0f/60;
    time.sim_scale = 1.0f;
    time.max_allowed_per_frame = 0.25f;

    //float accum = 0;

    window = os_create_window(1600, 800, "PepegaClap"_s);
    os_lock_mouse(window);

    reset_entities();
    
    init_renderer(window);

    while(true) {
        update_time();

        if(os_poll_events(window)) break;

        //accum += time.dt;
        //while(accum >= time.sim_dt) {
            simulate(time.sim_dt * time.sim_scale);
        //    accum -= time.sim_dt;
        //}

        //TODO: Stuttering comes from not doing position/rotation interpolation. Do that.
        //float lerp = accum / fixedDeltaTime;
        render(window);
#if 0
    printf_s("last frame: %f ms\n", time.dt * 1000);
    printf_s("FPS: %f\n", 1 / time.dt);
    fflush(stdout);   //CPU usage goes 10x and my laptops fan starts going crazy if we do this every frame. WTF!
#endif
    }

    end_renderer();
}
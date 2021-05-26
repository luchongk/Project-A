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
float max_frame_time = 0.25f;

static void update_time() {
    s64 now = os_get_timestamp();
    
    time.dt = (now - time.last_stamp) / (float)time.frequency;
    if(time.dt > max_frame_time) {
        //Framerate too low, we are falling behind in the simulation! D: Preventing spiral of death.
        time.dt = max_frame_time;
    }
    time.scaled_sim_dt = time.sim_dt * time.modifier;

    time.since_start = (now - time.start_stamp) / (float)time.frequency;
    time.last_stamp = now;
}

static void handle_input(OSWindow* window) {
    if(key_down('P')) {
        paused = !paused;
        os_lock_cursor(paused ? nullptr : window);
    }

    if(key_down('R')) {
        reset_entities();
        cubes_rotation = 0;
    }

    if(key_down(VK_LEFT)) {
        time.modifier *= 0.5f;
    }

    if(key_down(VK_RIGHT)) {
        time.modifier *= 2.0f;
    }

    if(key_down(VK_TAB)) {
        character = !character;
    }

    if(key_down(VK_F9)) {
        bool is_fullscreen = os_is_fullscreen(window);
        os_set_fullscreen(window, !is_fullscreen, false);
    }

    if(key_down(VK_F11)) {
        bool is_fullscreen = os_is_fullscreen(window);
        os_set_fullscreen(window, !is_fullscreen, true);
    }
}

void main() {
    time.start_stamp = os_get_timestamp();
    time.frequency = os_get_timer_frequency();
    time.sim_dt = 1.0f/60;
    time.modifier = 1.0f;
    max_frame_time = 0.25f;

    //float accum = 0;

    OSWindow* window = os_create_window(200, 100, 1600, 800);
    os_lock_cursor(window);

    reset_entities();
    
    init_renderer(window);

    while(true) {
        if(os_poll_events(window)) break;

        update_time();

        handle_input(window);

        //accum += time.dt;
        //while(accum >= time.sim_dt) {
            simulate(time.scaled_sim_dt);
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
#include "platform.h"
#include "main.h"

#ifdef _WIN32
    #include "platform_win32.cpp"
#endif

#include "stb_image.cpp"
#include "stb_truetype.cpp"
#include "input.cpp"
#include "render.cpp"
#include "simulation.cpp"
#include "graphics.cpp"
#include "obj_loader.cpp"
#include "entities.cpp"
#include "rect.cpp"
#include "font.cpp"
#include "simple_draw.cpp"
#include "ui.cpp"

Time time;
OSWindow* window;

static void update_time() {
    u64 now = os_get_timestamp();
    
    time.since_start = os_elapsed_time(time.start_stamp, now);
    time.dt          = os_elapsed_time(time.last_frame_stamp, now);
    
    if(time.dt > time.max_allowed_per_frame) {
        //Framerate too low, we are falling behind in the simulation! D: Preventing spiral of death.
        time.dt = time.max_allowed_per_frame;
    }

    time.last_frame_stamp = now;
}

void main() {
    time.start_stamp = os_get_timestamp();
    time.sim_dt = 1.0f/60;
    time.sim_scale = 1.0f;
    time.max_allowed_per_frame = 0.25f;

    //float accum = 0;

    window = os_create_window(1600, 800, "PepegaClap"_s);
    os_show_mouse(true);

    reset_scene();
    
    init_renderer(window);

    while(true) {
        wait_for_vblank();
        
        update_time();

        update_input();

        bool should_quit = handle_input(&events);
        if(should_quit) break;

        //accum += time.dt;
        //while(accum >= time.sim_dt) {
            simulate(time.dt);
        //    accum -= time.sim_dt;
        //}

        //TODO: Stuttering comes from not doing position/rotation interpolation. Do that.
        //float lerp = accum / fixedDeltaTime;
        render(window);

#if 0
    printf_s("last frame: %f ms\n", time.dt * 1000);
    printf_s("FPS: %f\n", 1 / time.dt);
    fflush(stdout);   //CPU usage goes 10x and my laptops fan starts going crazy if we do this every frame. WTF!
#elif 0
    printf_s("Mouse X: %f Y: %f\n", input.mouse_position.x, input.mouse_position.y);
    fflush(stdout);   //CPU usage goes 10x and my laptops fan starts going crazy if we do this every frame. WTF!
#endif
    }

    end_renderer();
}
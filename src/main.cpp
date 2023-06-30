#include "platform.h"
#include "main.h"

#ifdef _WIN32
    #include "platform_win32.cpp"
#endif

#include "stb_image.cpp"
#include "stb_truetype.cpp"
#include "input.cpp"
#include "render.cpp"
#include "physics.cpp"
#include "simulation.cpp"
#include "graphics_d3d11.cpp"
#include "obj_loader.cpp"
#include "entities.cpp"
#include "rect.cpp"
#include "font.cpp"
#include "simple_draw.cpp"
#include "ui.cpp"

Time my_time;
OSWindow* window;
LinearArena temporary_storage;

static void update_time() {
    u64 now = os_get_timestamp();
    
    my_time.since_start = os_elapsed_time(my_time.start_stamp, now);
    my_time.dt          = os_elapsed_time(my_time.last_frame_stamp, now);
    
    if(my_time.dt > my_time.max_dt_allowed) {
        //Framerate too low, we are falling behind in the simulation (or we had a big spike)! D: Preventing spiral of death by slowing down the simulation.
        my_time.dt = my_time.max_dt_allowed;
    }

    my_time.last_frame_stamp = now;
}

void main() {
    init_arena(&temporary_storage, megabytes(1), malloc(megabytes(1)));

    my_time.start_stamp = os_get_timestamp();
    my_time.sim_dt = 1.0f/60;
    my_time.sim_scale = 1.0f;
    my_time.max_dt_allowed = 0.25f;

    //float accum = 0;

    window = os_create_window(1600, 800, "PepegaClap"_s);
    os_set_fullscreen(window, true);
    
    reset_scene();
    
    init_renderer(window);

    wait_for_vblank();

    while(true) {
        reset_arena(&temporary_storage);

        update_time();

        //u64 before = os_get_timestamp();
        update_input(window);

        bool should_quit = handle_input(&events);
        if(should_quit) break;

        //accum += time.dt;
        //while(accum >= time.sim_dt) {
            simulate(my_time.dt);
        //    accum -= time.sim_dt;
        //}

        //TODO: Stuttering comes from not doing position/rotation interpolation. Do that.
        //float lerp = accum / fixedDeltaTime;
        render(window);

        /*float elapsed = os_elapsed_time(before, os_get_timestamp());
        if(elapsed > 0.002f) {
            printf_s("elapsed: %.4f\n", elapsed);
        }*/

        wait_for_vblank();

#if 0
    printf_s("last frame: %f ms\n", my_time.dt * 1000);
    printf_s("FPS: %f\n", 1 / my_time.dt);
#elif 0
    printf_s("Mouse X: %f Y: %f\n", input.mouse_position.x, input.mouse_position.y);
#endif
    }

    end_renderer();
}
#ifdef _WIN32
    #include "platform_win32.cpp"
#endif

#include "main.h"

bool do_step = false;

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
    my_time.simulation_dt = 1.0f/144;
    my_time.sim_scale = 1.0f;
    my_time.max_dt_allowed = 0.25f;

    float accum = 0;

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

        ui_update();
       
        auto sim_dt = my_time.simulation_dt;
        if(paused) {
            if(do_step) {
                for(int i = 0; i < count_Player; i++) {
                    auto mat = (MaterialBasic*)pool_Player[i].entity->material;
                    mat->diffuse = Vec3{0,1,0};
                }
                do_step = false;
                simulate(sim_dt);
            }
        }
        else {
            for(int i = 0; i < count_Player; i++) {
                auto mat = (MaterialBasic*)pool_Player[i].entity->material;
                mat->diffuse = Vec3{0,1,0};
            }
            // @Journey 07/05/2023: After struggling with this topic for a few years I think I finally got a good understanding of it.
            // The basics of the problem are described here: https://gafferongames.com/post/fix_your_timestep . (Come back after reading that)
            // It describes the problem well but the solution the article arrives to is not a good one in my opinion.
            // You'll see many people getting confused about it thinking that the writer is talking about extrapolating the currentState forward in time (and in some places it seems like even he thinks he is doing that).
            // What he is actually doing is getting the remainder of the accumulator (the time left to simulate for this frame) and using it to interpolate between the _second-to-last_ (what he calls "previousState") and the _last state_ (what he calls "currentState") calculated.
            // The problem is those states have nothing to do with whats left in the accumulator, since the time (dt) between both of those states was already subtracted from the accumulator.
            // So the effect of the algorithm he uses is essentially to smooth out the rendering AT THE COST of making the rendering always be 1 frame behind the simulation.
            // That means theres always 1 frame of added input latency just from doing this and it forces you to always store the previous state of the simulation along with the current state.
            // This does not seem like an acceptable solution to me.
            // 
            // Better solutions, the way I see it, are the ones provided in this explanation: https://youtu.be/fdAOPHgW7qM (fast-forward to 1:48:46 if you want to skip the discussion of the problems of not handling this correctly)
            // The are 2 proposed solutions, one that results in deterministic simulation and one that doesn't:
            // 
            // 1) The easiest one (and the one I'm using) is to use a fixed time-step (sim_dt in my code) to divide the time between frames just as in the gafferongames article, but instead of doing the backwards interpolation with
            // the remainder, we just call our same simulation function with that remainder. This makes the simulation technically non-deterministic; but in practice, since the remainder will always be smaller than the fixed time-step
            // (and smaller time-steps generally mean less deviation from the intended results), then the error between the deterministic simulation (The hypothetical case were the remainder is always 0) and the non-deterministic will
            // be small. How small? Well, it depends on the player's framerate and the size of the fixed time-step used (smaller time-steps -> smaller deviation). Since I don't really care about making the simulation deterministic,
            // this is the solution I'm using for now.
            //
            // 2) If we really cared about determinism we could instead use a fixed time-step as before, but instead of using the remainder to run the actual simulation that changes the state of the world, we would run a throwaway simulation
            // just for knowing where to render the objects and discard those changes after. We would then keep that remainder in the accumulator to be re-simulated in the next frame to keep simulation consistent.
            // A potential problem that I can think for this solution is that player input for the throwaway will be different from next frame's real simulation input, since input read usally happens after render and before update.
            // There might be some complicated way to correlate inputs to simulation steps but I haven't put much thought into it. At least for pre-recorded input this solution should play the same every time it's run.

            accum += my_time.dt * my_time.sim_scale;
            while(accum >= sim_dt) {
                simulate(sim_dt);
                accum -= sim_dt;
            }

            // This part is not in the video, but I'm testing the idea of not running the simulation until next frame if the remainder is too small.
            // It should make things a bit more robust without having too much impact on the visuals.
            //if(accum > 0.0001f) {
              simulate(accum);
              accum = 0;
            //}
        }

        update_camera(my_time.dt);

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
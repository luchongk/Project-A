#include <cstdio>
#include "windowing.h"
#include "times.h"
#include "entities.h"
#include "render.h"
#include "input.h"
#include "handle_input.h"
#include "simulation.h"
#include "basic.h"
#include "general.h"
#include "ui.h"
#include "graphics/d3d11/utils.h"
#include "font.h"

const float ORTHOGRAPHIC_VIEW_WIDTH = 20.0f;    // In world units.

Time my_time;
OsWindow the_window;
bool do_step;
bool use_perspective = true;

FontHandle inconsolata;

static void update_time() {
    u64 now = get_timestamp();
    
    my_time.since_start = get_elapsed_time(my_time.start_stamp, now);
    my_time.dt          = get_elapsed_time(my_time.last_frame_stamp, now);
    
    if(my_time.dt > my_time.max_dt_allowed) {
        //Framerate too low, we are falling behind in the simulation (or we had a big spike)! D: Preventing spiral of death by slowing down the simulation.
        my_time.dt = my_time.max_dt_allowed;
    }

    my_time.last_frame_stamp = now;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    setvbuf(stdout, nullptr, _IONBF, 0);
    SetCurrentDirectoryA("..\\");
    
    my_time.start_stamp = get_timestamp();
    my_time.simulation_dt = 1.0f/144;
    my_time.sim_scale = 1.0f;
    my_time.max_dt_allowed = 0.25f;

    the_window = create_window(400, 400, 1600, 800, "PepegaClap"_s, false);
    set_window_min_size(the_window, 500, 500); 
    set_fullscreen(the_window, true);
    
    init_renderer();

    inconsolata = font_load_from_file("assets\\fonts\\Inconsolata.ttf"_s);
    ui_init(2560, 1440);

    reset_scene();

    ShowWindow(the_window, SW_SHOW);
    flush_os_events();

    float accum = 0;
    while(true) {
        wait_for_present(the_window);

        update_time();

        gather_os_events();
        bool should_quit = handle_input();
        flush_os_events();
        if(should_quit) break;
       
        auto sim_dt = my_time.simulation_dt;
        if(paused) {
            if(do_step) {
                simulate(sim_dt);
                do_step = false;
            }
        }
        else {
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
            // just for knowing where to render the objects and discard those changes after. We would then keep that remainder in the accumulator to be re-simulated in the next frame to keep the simulation consistent.
            // A potential problem that I can think for this solution is that player input for the throwaway will be different from next frame's real simulation input, since input reading usally happens after render() and before update().
            // There might be some complicated way to correlate inputs to simulation steps but I haven't put much thought into it. At least for pre-recorded input this solution should play the same every time it's run.

            accum += my_time.dt * my_time.sim_scale;
            while(accum >= sim_dt) {
                simulate(sim_dt);
                accum -= sim_dt;
            }

            // This part is not in the video, but I'm testing the idea of not running the simulation until next frame if the remainder is too small.
            // It should make things a bit more robust without having too much impact on the visuals.
            if(accum > 0.0001f) {
                simulate(accum);
                accum = 0;
            }
        }

        update_camera(my_time.dt);

        render();
        ui_render_pass();

        swap_buffers(the_window);

        /*float elapsed = os_elapsed_time(before, os_get_timestamp());
        if(elapsed > 0.002f) {
            printf_s("elapsed: %.4f\n", elapsed);
        }*/

        arena_clear(temp_arena);

#if 0
    printf_s("last frame: %f ms\n", my_time.dt * 1000);
    printf_s("FPS: %f\n", 1 / my_time.dt);
#elif 0
    printf_s("Mouse X: %f Y: %f\n", input.mouse_position.x, input.mouse_position.y);
#endif
    }

    font_release(inconsolata);
    free_font_atlas();
    end_renderer();
}

ArrayView<GlyphInfo> ui_text(FontHandle font, int size, String text, int x_padding = 0) {
    auto run = get_glyph_run(font, size, text, temp_allocator);
    float width = 0;
    float height = run[0].subregion.h;
    For(run) width += it.advance;
    
    ui_element(); w_px(width + 2 * x_padding) h_px(height)
    return run;
}

void draw_text(Rect rect, ArrayView<GlyphInfo> glyphs, int x_padding = 0, Vector4 color = {1,1,1,1}) {
    float x = floorf(rect.x + x_padding);
    For(glyphs) {
        sd_draw_rect({x + it.x_offset, rect.y, it.subregion.w, it.subregion.h}, it.uv, it.atlas_texture, color);
        x += it.advance;
    }
}

bool devtools_open = true;
void ui_declare() {
    ui_enter_data_scope("devtools"_s); {
        if(devtools_open) {
            auto e = ui_begin_element(); w_fit _pad_x(10) _pad_top(10) h_expand(1) _column _gap(5) {
                sd_draw_rect(e->rect, {0,0.1f,0.2f,0.6f});

                auto glyphs = ui_text(inconsolata, 24, "Reset Scene!"_s, 8);
                auto b_reset = ui_interactable(current_element->rect);
                auto background = lerp(Vector3{0.1f, 0.1f, 0.1f}, Vector3{0.2f, 0.2f, 0.2f}, b_reset->hover_t);
                background = lerp(background, Vector3{0.01f, 0.01f, 0.01f}, b_reset->hold_t);
                sd_draw_rect(current_element->rect, background, 8, false);
                draw_text(current_element->rect, glyphs, 8);
                if(on_click(b_reset)) reset_scene();

                auto pos = main_player->entity->position;
                auto pos_string = sprint("Player position: (%f, %f, %f)", temp_allocator, pos.x, pos.y, pos.z);
                glyphs = ui_text(inconsolata, 18, pos_string);
                draw_text(current_element->rect, glyphs);
            } ui_end_element();
        }
    }
}
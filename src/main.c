#include <stdio.h>
#include <float.h>

#define MSF_GIF_IMPL
#include "../include/msf_gif.h"

#include "shit-game-engine.h"
#include "../include/systems.h"
#include "../include/components.h"
#include "../include/game_input_actions.h"

int main(int argc, char **argv) {
    const Vector2i WINDOW_RESOLUTION = {1280, 1024};

    bool gif_recording = false;
    MsfGifState gifState = {0};
    uint8_t *gif_pixels;
    char *gif_name;
    int width = WINDOW_RESOLUTION.x, height = WINDOW_RESOLUTION.y, centisecondsPerFrame = 2, quality = 16;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--gif") == 0) {
            gif_recording = true;
            msf_gif_begin(&gifState, width, height);
            gif_pixels = malloc(width * height * 4);
            gif_name = argv[i + 1];
        }
    }

    const char *WINDOW_TITLE = gif_recording ? "Kool Kubez (Recording GIF)" : "Kool Kubez";


    Scene current_scene = initialize_scene();

    AppContext app_context;

    if (initialize_sdl_components(&app_context, WINDOW_RESOLUTION, WINDOW_TITLE) != 0) {
        fprintf(stderr, "Error initializing SDL components\n");
        return 1;
    }

    app_context.depth_buffer = malloc(sizeof(DepthBuffer));
    app_context.depth_buffer->width = app_context.window_resolution.x;
    app_context.depth_buffer->height = app_context.window_resolution.y;
    app_context.depth_buffer->depth_values = malloc(WINDOW_RESOLUTION.x * WINDOW_RESOLUTION.y * sizeof(float));

    Uint64 ticks_now = SDL_GetPerformanceCounter();
    Uint64 ticks_last = 0;
    app_context.delta_time = 0;
    app_context.time_accumulator = 0;

    ORBIT = register_component(&current_scene, sizeof(OrbitComponent), "orbit", parse_orbit_component);
    TUMBLE = register_component(&current_scene, sizeof(TumbleComponent), "tumble", parse_tumble_component);
    register_system(&current_scene, orbit_system, (1ULL << TRANSFORM) | (1ULL << ORBIT));
    register_system(&current_scene, tumble_system, (1ULL << TRANSFORM) | (1ULL << TUMBLE));
    register_system(&current_scene, test_camera_orbit_system, 0);

    register_game_input_actions(&app_context.input_action_map);

    load_scene_from_file(&current_scene, "scenes/test_scene.json");


    while (app_context.application_running) {
        app_context.delta_time = get_delta_time(&ticks_now, &ticks_last);

        //double frame_rate = get_instantaneous_frame_rate(&ticks_now, &ticks_last);

        update_input_actions(&app_context.input_action_map, &app_context);

        handle_sdl_events(&app_context);

        // To-do move this elsewhere later
        app_context.time_accumulator += app_context.delta_time;
        if (app_context.time_accumulator > 0.0167 ) {
            app_context.time_accumulator = 0;
        }

        /**
        run_systems(&current_scene, app_context.delta_time);
        RenderList render_list = generate_render_list(&current_scene, &app_context);
        render(&app_context, &render_list);
        */

        for (int i = 0; i < app_context.window_resolution.x * app_context.window_resolution.y; i++) {
            app_context.depth_buffer->depth_values[i] = FLT_MAX; // Reset depth buffer values to maximum depth
        }

        Uint64 t0 = SDL_GetPerformanceCounter();
        run_systems(&current_scene, &app_context);
        Uint64 t1 = SDL_GetPerformanceCounter();
        RenderList render_list = generate_render_list(&current_scene, &app_context);
        Uint64 t2 = SDL_GetPerformanceCounter();
        render(&app_context, &render_list);
        Uint64 t3 = SDL_GetPerformanceCounter();

        double freq = SDL_GetPerformanceFrequency();
        static int frame = 0;
        if (frame++ % 60 == 0) {
            printf("systems: %.2fms  pipeline: %.2fms  render: %.2fms  tris: %d\n",
                (t1-t0)/freq*1000, (t2-t1)/freq*1000, (t3-t2)/freq*1000,
                render_list.triangle_count);
        }

        if (gif_recording) {
            SDL_RenderReadPixels(app_context.renderer, NULL, SDL_PIXELFORMAT_RGBA32, gif_pixels, width * 4);
            msf_gif_frame(&gifState, gif_pixels, centisecondsPerFrame, quality, width * 4);
        }
    }

    if (gif_recording) {
        MsfGifResult result = msf_gif_end(&gifState);
        if (result.data) {
            FILE * fp = fopen(gif_name, "wb");
            fwrite(result.data, result.dataSize, 1, fp);
            fclose(fp);
        }
        msf_gif_free(result);
        free(gif_pixels);
    }

    cleanup_sdl_components(&app_context);
    return 0;
}
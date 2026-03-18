#include "shit-game-engine.h"
#include "../include/systems.h"
#include "../include/components.h"
#include "../include/game_input_actions.h"

int main(int argc, char **argv) {
    bool record_gif = false;
    const char* gif_path;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--gif") == 0) {
            record_gif = true;
            gif_path = argv[i + 1];
            break;
        }
    }

    AppContext app = {0};
    engine_init(&app, &(EngineRunConfig){
        .window_title = "Kool Azz Kubez",
        .window_resolution = (Vector2i){1280, 1024},
        .rasterizer_tile_size = 32,
        .record_gif = record_gif,
        .gif_path = record_gif ? gif_path : NULL,
        .log_mode = LOG_MODE_STDOUT,
        .log_verbosity = LOG_VERBOSITY_DEBUG
    });

    Scene game_scene = {0};

    ORBIT = register_component(&game_scene, sizeof(OrbitComponent), "orbit", parse_orbit_component);
    TUMBLE = register_component(&game_scene, sizeof(TumbleComponent), "tumble", parse_tumble_component);
    engine_load_scene(&game_scene, "scenes/zaychyka.json");
    register_system(&game_scene, orbit_system, (1ULL << TRANSFORM) | (1ULL << ORBIT));
    register_system(&game_scene, tumble_system, (1ULL << TRANSFORM) | (1ULL << TUMBLE));
    register_system(&game_scene, test_camera_freelook_system, 0);
    //register_system(&game_scene, test_camera_orbit_system, 0);
    register_system(&game_scene, test_light_movement_system, 0);

    register_game_input_actions(&app.input_action_map);

    while (app.application_running) {
        engine_frame_start(&app);
        engine_run(&app, &game_scene);
        engine_frame_end(&app);
    }

    engine_shutdown(&app);
}
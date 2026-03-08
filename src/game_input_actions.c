#include "shit-game-engine.h"



void register_game_input_actions(InputActionMap *map) {
    InputBinding move_right[] = {{INPUT_KEYBOARD, SDL_SCANCODE_D}};
    InputBinding move_left[]  = {{INPUT_KEYBOARD, SDL_SCANCODE_A}};
    register_axis_1d_input_action(map, "camera_orbit_horizontal", (AxisBindings){move_right, 1}, (AxisBindings){move_left, 1});

    InputBinding scroll_up[]   = {{INPUT_MOUSE_SCROLL_UP, 0}};
    InputBinding scroll_down[] = {{INPUT_MOUSE_SCROLL_DOWN, 0}};
    register_axis_1d_input_action(map, "camera_zoom", (AxisBindings){scroll_up, 1}, (AxisBindings){scroll_down, 1});
}

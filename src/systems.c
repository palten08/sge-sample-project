#include <SDL2/SDL.h>

#include "../include/systems.h"
#include "types.h"
#include "matrix_operations.h"
#include "quaternion_operations.h"
#include "vector_operations.h"
#include "ecs.h"
#include "../include/components.h"
#include "../include/input_actions.h"

void orbit_system(Scene *scene, AppContext *app_context) {
    uint64_t required = (1ULL << TRANSFORM) | (1ULL << ORBIT);
    for (int e = 0; e < scene->registered_entity_count; e++) {
        if ((scene->component_masks[e] & required) != required) continue;
        TransformComponent *transform_component = get_component(scene, TRANSFORM, e);
        OrbitComponent *orbit_component = get_component(scene, ORBIT, e);

        // Orbit
        orbit_component->angle += orbit_component->speed * app_context->delta_time;
        orbit_component->angle = fmodf(orbit_component->angle, 2.0f * M_PI);
        float angle = orbit_component->angle + orbit_component->phase;

        transform_component->position.x = orbit_component->center.x + orbit_component->radius * sin(angle);
        transform_component->position.z = orbit_component->center.z + orbit_component->radius * cos(angle);

        // Rebuild model matrix
        Matrix4 translation = mat4_create_translation_matrix(transform_component->position.x, transform_component->position.y, transform_component->position.z);
        Matrix4 rx = mat4_create_rotation_x_matrix(transform_component->rotation.x);
        Matrix4 ry = mat4_create_rotation_y_matrix(transform_component->rotation.y);
        Matrix4 rz = mat4_create_rotation_z_matrix(transform_component->rotation.z);
        Matrix4 rotation = mat4_multiply(rz, mat4_multiply(ry, rx));
        Matrix4 scale = mat4_create_scaling_matrix(transform_component->scale.x, transform_component->scale.y, transform_component->scale.z);
        transform_component->model_matrix = mat4_multiply(translation, mat4_multiply(rotation, scale));
    }
}

void tumble_system(Scene *scene, AppContext *app_context) {
    uint64_t required = (1ULL << TRANSFORM) | (1ULL << TUMBLE);
    for (int e = 0; e < scene->registered_entity_count; e++) {
        if ((scene->component_masks[e] & required) != required) continue;
        TransformComponent *transform_component = get_component(scene, TRANSFORM, e);
        TumbleComponent *tumble_component = get_component(scene, TUMBLE, e);

        // Replace with quaternion rotation
        float angle = tumble_component->speed * app_context->delta_time;
        Quaternion delta = quaternion_from_axis_angle(
            tumble_component->direction.x,
            tumble_component->direction.y,
            tumble_component->direction.z,
            angle
        );
        transform_component->rotation = quaternion_multiply(delta, transform_component->rotation);
        Matrix4 rotation_matrix = quaternion_to_matrix4(transform_component->rotation);
        Matrix4 translation = mat4_create_translation_matrix(transform_component->position.x, transform_component->position.y, transform_component->position.z);
        Matrix4 scale = mat4_create_scaling_matrix(transform_component->scale.x, transform_component->scale.y, transform_component->scale.z);
        transform_component->model_matrix = mat4_multiply(translation, mat4_multiply(rotation_matrix, scale));
    }
}

void test_camera_orbit_system(Scene *scene, AppContext *app_context) {
    static float radius = -1.0f;
    static float height;
    static float camera_angle;

    static float zoom_velocity = 0.0f;
    float zoom_strength = 5.0f;
    float min_radius = 2.0f;
    float max_radius = 100.0f;

    static float orbit_speed = 0.0f;

    if (radius < 0) {
        float dx = scene->virtual_camera.position.x - scene->virtual_camera.look_target.x;
        float dz = scene->virtual_camera.position.z - scene->virtual_camera.look_target.z;
        radius = sqrtf(dx * dx + dz * dz);
        height = scene->virtual_camera.position.y;
        camera_angle = atan2f(dx, dz);
    }

    static float orbit_velocity = 0.0f;
    float orbit_strength = 0.02f;

    int zoom_action_index = get_input_action_index_by_name(&app_context->input_action_map, "camera_zoom");
    if (zoom_action_index < 0) return;
    float scroll = get_input_action_axis_1d_value(&app_context->input_action_map.input_actions[zoom_action_index]);
    zoom_velocity -= scroll * zoom_strength;


    radius += zoom_velocity * app_context->delta_time;
    zoom_velocity *= powf(0.05f, app_context->delta_time);
    radius = fmaxf(min_radius, fminf(radius, max_radius));

    int action_index = get_input_action_index_by_name(&app_context->input_action_map, "camera_orbit_horizontal");
    if (action_index < 0) return;
    float axis = get_input_action_axis_1d_value(&app_context->input_action_map.input_actions[action_index]);
    orbit_velocity += axis * orbit_strength;

    camera_angle += orbit_velocity * app_context->delta_time;
    orbit_velocity *= powf(0.05f, app_context->delta_time);

    scene->virtual_camera.position.x = scene->virtual_camera.look_target.x + radius * sinf(camera_angle);
    scene->virtual_camera.position.z = scene->virtual_camera.look_target.z + radius * cosf(camera_angle);
    scene->virtual_camera.position.y = height;

    scene->virtual_camera.view_matrix = mat4_create_look_at_matrix(
        scene->virtual_camera.position,
        scene->virtual_camera.look_target,
        (Vector3f){0.0f, 1.0f, 0.0f}
    );
}

void test_light_movement_system(Scene *scene, AppContext *app_context) {
    static float angle = 0.0f;
    float speed = 0.6f; // Radians per second
    angle += speed * app_context->delta_time;
    scene->directional_light.direction.x = sinf(angle);
    scene->directional_light.direction.z = cosf(angle);
}

void test_camera_freelook_system(Scene *scene, AppContext *app_context) {
    static float yaw = 0.0f;
    static float pitch = 0.0f;
    static bool initialized = false;

    if (!initialized) {
        Vector3f dir = vec3f_normalize(vec3f_subtract(scene->virtual_camera.look_target, scene->virtual_camera.position));
        yaw = atan2f(dir.x, dir.z);
        pitch = asinf(fmaxf(-1.0f, fminf(dir.y, 1.0f)));
        initialized = true;
        return;
    }

    // Mouse look — only when right mouse button held
    int mouse_dx, mouse_dy;
    Uint32 buttons = SDL_GetRelativeMouseState(&mouse_dx, &mouse_dy);
    if (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
        yaw -= mouse_dx * 0.003f;
        pitch -= mouse_dy * 0.003f;
        if (pitch > 1.4f) pitch = 1.4f;
        if (pitch < -1.4f) pitch = -1.4f;
    }

    Vector3f forward = {
        sinf(yaw) * cosf(pitch),
        sinf(pitch),
        cosf(yaw) * cosf(pitch)
    };

    float move_speed = 5.0f * fminf(app_context->delta_time, 0.1f);
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_W]) scene->virtual_camera.position = vec3f_add(scene->virtual_camera.position, vec3f_multiply_scalar(forward, move_speed));
    if (keys[SDL_SCANCODE_S]) scene->virtual_camera.position = vec3f_add(scene->virtual_camera.position, vec3f_multiply_scalar(forward, -move_speed));

    Vector3f right = vec3f_normalize(vec3f_cross_product(forward, (Vector3f){0.0f, 1.0f, 0.0f}));
    if (keys[SDL_SCANCODE_D]) scene->virtual_camera.position = vec3f_add(scene->virtual_camera.position, vec3f_multiply_scalar(right, move_speed));
    if (keys[SDL_SCANCODE_A]) scene->virtual_camera.position = vec3f_add(scene->virtual_camera.position, vec3f_multiply_scalar(right, -move_speed));
    if (keys[SDL_SCANCODE_SPACE]) scene->virtual_camera.position = vec3f_add(scene->virtual_camera.position, (Vector3f){0, move_speed, 0});
    if (keys[SDL_SCANCODE_LSHIFT]) scene->virtual_camera.position = vec3f_add(scene->virtual_camera.position, (Vector3f){0, -move_speed, 0});

    scene->virtual_camera.look_target = vec3f_add(scene->virtual_camera.position, forward);
    scene->virtual_camera.view_matrix = mat4_create_look_at_matrix(scene->virtual_camera.position, scene->virtual_camera.look_target, (Vector3f){0.0f, 1.0f, 0.0f});
}
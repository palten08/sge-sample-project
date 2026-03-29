#include <SDL3/SDL.h>
#include <math.h>
#include <stdlib.h>

#include "../include/systems.h"
#include "types.h"
#include "matrix_operations.h"
#include "quaternion_operations.h"
#include "vector_operations.h"
#include "ecs.h"
#include "../include/components.h"
#include "../include/input_actions.h"

void orbit_system(Scene *scene, AppContext *app_context) {
    uint64_t required_bitmask = (1ULL << TRANSFORM) | (1ULL << ORBIT);
    MatchedArchetypes matched_archetypes = find_matching_archetypes(scene, required_bitmask);
    if (matched_archetypes.archetype_count == 0) {
        //LOG_DEBUG("No archetypes matched for orbit system");
        free(matched_archetypes.archetypes);
        return;
    }
    for (int i = 0; i < matched_archetypes.archetype_count; i++) {
        TransformComponent *transform_column = get_archetype_column_pointer(matched_archetypes.archetypes[i], TRANSFORM);
        OrbitComponent *orbit_column = get_archetype_column_pointer(matched_archetypes.archetypes[i], ORBIT);
        for (int row = 0; row < matched_archetypes.archetypes[i]->row_count; row++) {
            // Orbit
            orbit_column[row].angle += orbit_column[row].speed * app_context->delta_time;
            orbit_column[row].angle = fmodf(orbit_column[row].angle, 2.0f * M_PI);
            float angle = orbit_column[row].angle + orbit_column[row].phase;

            transform_column[row].position.x = orbit_column[row].center.x + orbit_column[row].radius * sinf(angle);
            transform_column[row].position.z = orbit_column[row].center.z + orbit_column[row].radius * cosf(angle);

            // Rebuild model matrix
            Matrix4 translation = mat4_create_translation_matrix(transform_column[row].position.x, transform_column[row].position.y, transform_column[row].position.z);
            Matrix4 rotation = quaternion_to_matrix4(transform_column[row].rotation);
            Matrix4 scale = mat4_create_scaling_matrix(transform_column[row].scale.x, transform_column[row].scale.y, transform_column[row].scale.z);
            transform_column[row].model_matrix = mat4_multiply(translation, mat4_multiply(rotation, scale));
        }
    }
    free(matched_archetypes.archetypes);
}

void tumble_system(Scene *scene, AppContext *app_context) {
    uint64_t required_bitmask = (1ULL << TRANSFORM) | (1ULL << TUMBLE);
    MatchedArchetypes matched_archetypes = find_matching_archetypes(scene, required_bitmask);
    if (matched_archetypes.archetype_count == 0) {
        LOG_DEBUG("No archetypes matched for tumble system");
        free(matched_archetypes.archetypes);
        return;
    }
    for (int i = 0; i < matched_archetypes.archetype_count; i++) {
        TransformComponent *transform_column = get_archetype_column_pointer(matched_archetypes.archetypes[i], TRANSFORM);
        TumbleComponent *tumble_column = get_archetype_column_pointer(matched_archetypes.archetypes[i], TUMBLE);
        for (int row = 0; row < matched_archetypes.archetypes[i]->row_count; row++) {
            // Tumble
            float angle = tumble_column[row].speed * app_context->delta_time;
            Quaternion delta = quaternion_from_axis_angle(tumble_column[row].direction.x, tumble_column[row].direction.y, tumble_column[row].direction.z, angle);
            transform_column[row].rotation = quaternion_multiply(delta, transform_column[row].rotation);

            Matrix4 rotation_matrix = quaternion_to_matrix4(transform_column[row].rotation);
            Matrix4 translation_matrix = mat4_create_translation_matrix(transform_column[row].position.x, transform_column[row].position.y, transform_column[row].position.z);
            Matrix4 scale_matrix = mat4_create_scaling_matrix(transform_column[row].scale.x, transform_column[row].scale.y, transform_column[row].scale.z);
            transform_column[row].model_matrix = mat4_multiply(translation_matrix, mat4_multiply(rotation_matrix, scale_matrix));
        }
    }
    free(matched_archetypes.archetypes);
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

    int horizontal_freelook_action_index = get_input_action_index_by_name(&app_context->input_action_map, "camera_freelook_horizontal");
    int vertical_freelook_action_index = get_input_action_index_by_name(&app_context->input_action_map, "camera_freelook_vertical");
    int move_action_index = get_input_action_index_by_name(&app_context->input_action_map, "camera_move");
    int freelook_action_index = get_input_action_index_by_name(&app_context->input_action_map, "camera_freelook");

    if (!initialized) {
        Vector3f dir = vec3f_normalize(vec3f_subtract(scene->virtual_camera.look_target, scene->virtual_camera.position));
        yaw = atan2f(dir.x, dir.z);
        pitch = asinf(fmaxf(-1.0f, fminf(dir.y, 1.0f)));
        initialized = true;
        return;
    }

    // Mouse look — only when right mouse button held
    if (is_input_action_held(&app_context->input_action_map.input_actions[freelook_action_index])) {
        yaw -= get_input_action_axis_1d_value(&app_context->input_action_map.input_actions[horizontal_freelook_action_index]) * 0.001f;
        pitch -= get_input_action_axis_1d_value(&app_context->input_action_map.input_actions[vertical_freelook_action_index]) * 0.001f;
        if (pitch > 1.4f) pitch = 1.4f;
        if (pitch < -1.4f) pitch = -1.4f;
    }

    Vector3f forward = {
        sinf(yaw) * cosf(pitch),
        sinf(pitch),
        cosf(yaw) * cosf(pitch)
    };

    float move_speed = 5.0f * fminf(app_context->delta_time, 0.1f);

    if (is_input_action_held(&app_context->input_action_map.input_actions[move_action_index])) {
        Vector2f move_direction = get_input_action_axis_2d_value(&app_context->input_action_map.input_actions[move_action_index]);
        scene->virtual_camera.position = vec3f_add(scene->virtual_camera.position, vec3f_multiply_scalar(forward, move_direction.x * move_speed));
        Vector3f right = vec3f_normalize(vec3f_cross_product(forward, (Vector3f){0.0f, 1.0f, 0.0f}));
        scene->virtual_camera.position = vec3f_add(scene->virtual_camera.position, vec3f_multiply_scalar(right, move_direction.y * move_speed));
    }

    scene->virtual_camera.look_target = vec3f_add(scene->virtual_camera.position, forward);
    scene->virtual_camera.view_matrix = mat4_create_look_at_matrix(scene->virtual_camera.position, scene->virtual_camera.look_target, (Vector3f){0.0f, 1.0f, 0.0f});
}

void test_get_entity_record_system(Scene *scene, AppContext *app_context) {
    int debug_entity_record_action_index = get_input_action_index_by_name(&app_context->input_action_map, "debug_entity_record");
    if (debug_entity_record_action_index < 0) return;
    if (is_input_action_pressed(&app_context->input_action_map.input_actions[debug_entity_record_action_index])) {
        EntityRecord record = get_entity_record(scene, 0); // Assuming entity 0 exists
        LOG_DEBUG("EntityRecord for entity 0: archetype_index=%d, archetype_row_index=%d", record.archetype_index, record.archetype_row_index);
    }
}

void test_add_tumble_to_entity_system(Scene *scene, AppContext *app_context) {
    int add_component_action_index = get_input_action_index_by_name(&app_context->input_action_map, "debug_add_component");
    if (add_component_action_index < 0) return;
    if (is_input_action_pressed(&app_context->input_action_map.input_actions[add_component_action_index])) {
        int torus_entity_id = get_entity_id_by_name(scene, "archetyped_torus");
        if (get_component(scene, TUMBLE, torus_entity_id) != NULL) {
            LOG_DEBUG("Entity 'archetyped_torus' already has a tumble component");
            remove_component_from_entity(scene, torus_entity_id, TUMBLE);
            return;
        } else {
            LOG_DEBUG("Adding tumble component to entity 'archetyped_torus'");
            TumbleComponent tumble = {0};
            tumble.direction = (Vector3f){1.0f, 1.0f, 0.0f};
            tumble.speed = 2.0f;
            add_component(scene, torus_entity_id, TUMBLE, &tumble);
        }
    }
}

void test_entity_system(Scene *scene, AppContext *app_context) {
    int destroy_entity_action_index = get_input_action_index_by_name(&app_context->input_action_map, "debug_destroy_entity");
    if (destroy_entity_action_index < 0) return;
    if (is_input_action_pressed(&app_context->input_action_map.input_actions[destroy_entity_action_index])) {
        int torus_entity_id = get_entity_id_by_name(scene, "archetyped_torus");
        if (torus_entity_id < 0) {
            // Try to add the Torus back with all the components it had originally to test archetype assignment on creation as well
            LOG_DEBUG("Entity 'archetyped_torus' not found, creating it again");
            int new_entity_id = register_entity(scene, "archetyped_torus");
            TransformComponent transform = {0};
            transform.position = (Vector3f){3.3f, 0.0f, 1.3f};
            transform.rotation = (Quaternion){0.0f, 0.0f, 0.0f, 1.0f};
            transform.scale = (Vector3f){1.0f, 1.0f, 1.0f};
            add_component(scene, new_entity_id, TRANSFORM, &transform);
            TumbleComponent tumble = {0};
            tumble.direction = (Vector3f){2.0f, 1.0f, 5.0f};
            tumble.speed = 1.0f;
            add_component(scene, new_entity_id, TUMBLE, &tumble);
            MeshComponent mesh = {0};
            int torus_mesh_id = get_mesh_id_by_name(scene, "Torus");
            if (torus_mesh_id < 0) {
                LOG_ERROR("Torus mesh not found in asset library, cannot add MeshComponent to 'archetyped_torus' entity");
                return;
            }
            LOG_DEBUG("Found Torus mesh with ID %d, adding MeshComponent to 'archetyped_torus' entity", torus_mesh_id);
            mesh.mesh_id = torus_mesh_id;
            add_component(scene, new_entity_id, MESH, &mesh);
        } else {
            LOG_DEBUG("Destroying entity 'archetyped_torus' with ID %d", torus_entity_id);
            destroy_entity(scene, torus_entity_id);
        }
    }
} 
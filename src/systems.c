#include "../include/systems.h"
#include "types.h"
#include "matrix_operations.h"
#include "ecs.h"
#include "../include/components.h"

void orbit_system(Scene *scene, double delta_time) {
    for (int e = 0; e < scene->registered_entity_count; e++) {
        TransformComponent *transform_component = get_component(scene, TRANSFORM, e);
        OrbitComponent *orbit_component = get_component(scene, ORBIT, e);
        if (!transform_component || !orbit_component) continue;

        // Orbit
        orbit_component->angle += orbit_component->speed * delta_time;
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

void tumble_system(Scene *scene, double delta_time) {
    for (int e = 0; e < scene->registered_entity_count; e++) {
        TransformComponent *transform_component = get_component(scene, TRANSFORM, e);
        TumbleComponent *tumble_component = get_component(scene, TUMBLE, e);
        if (!transform_component || !tumble_component) continue;

        transform_component->rotation.x += tumble_component->speed * delta_time * tumble_component->direction.x;
        transform_component->rotation.y += tumble_component->speed * delta_time * tumble_component->direction.y;
        transform_component->rotation.z += tumble_component->speed * delta_time * tumble_component->direction.z;

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

void test_camera_orbit_system(Scene *scene, double delta_time) {
    static float radius = -1.0f;
    static float height;

    if (radius < 0) {
        // First call: capture the starting distance and height
        float dx = scene->virtual_camera.position.x - scene->virtual_camera.look_target.x;
        float dz = scene->virtual_camera.position.z - scene->virtual_camera.look_target.z;
        radius = sqrtf(dx * dx + dz * dz);
        height = scene->virtual_camera.position.y;
    }

    static float angle = 0.0f;
    angle += 0.3f * delta_time;
    scene->virtual_camera.position.x = scene->virtual_camera.look_target.x + radius * 1.1f * sin(angle);
    scene->virtual_camera.position.z = scene->virtual_camera.look_target.z + radius * 1.3f * cos(angle);
    scene->virtual_camera.position.y = height;

    scene->virtual_camera.view_matrix = mat4_create_look_at_matrix(
        scene->virtual_camera.position,
        scene->virtual_camera.look_target,
        (Vector3f){0.0f, 1.0f, 0.0f}
    );
}
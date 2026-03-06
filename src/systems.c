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
        transform_component->position.x = orbit_component->center.x + orbit_component->radius * sin(SDL_GetTicks() / 1000.0f * orbit_component->speed);
        transform_component->position.z = orbit_component->center.z + orbit_component->radius * 3 * -cos(SDL_GetTicks() / 1000.0f * orbit_component->speed);

        // Tumble
        transform_component->rotation.x += 1.0f * delta_time;
        transform_component->rotation.y += 1.0f * delta_time;
        transform_component->rotation.z += 1.0f * delta_time;

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
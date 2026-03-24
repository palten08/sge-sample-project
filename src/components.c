#include "../include/components.h"
#include "types.h"
#include "ecs.h"
#include "scene.h"

int ORBIT;
int TUMBLE;

void parse_orbit_component(Scene *scene, Entity entity, int component_id, void *data) {
    OrbitComponent *orbit_component = get_component(scene, component_id, entity);
    if (!orbit_component) {
        return; // Failed to get the component
    }
    orbit_component->radius = data ? ((OrbitComponent *)data)->radius : 0.0f;
    orbit_component->speed = data ? ((OrbitComponent *)data)->speed : 0.0f;
    orbit_component->phase = data ? ((OrbitComponent *)data)->phase : 0.0f;
    orbit_component->center.x = data ? ((OrbitComponent *)data)->center.x : 0.0f;
    orbit_component->center.y = data ? ((OrbitComponent *)data)->center.y : 0.0f;
    orbit_component->center.z = data ? ((OrbitComponent *)data)->center.z : 0.0f;
}

void parse_tumble_component(Scene *scene, Entity entity, int component_id, void *data) {
    TumbleComponent *tumble_component = get_component(scene, component_id, entity);
    if (!tumble_component) {
        return; // Failed to get the component
    }
    // Set defaults
    tumble_component->speed = 0.0f;
    tumble_component->direction = (Vector3f){0.0f, 0.0f, 0.0f};

    uint8_t *data_pointer = (uint8_t *)data;
    uint32_t property_count;
    memcpy(&property_count, data_pointer, sizeof(uint32_t));
    data_pointer += sizeof(uint32_t);
    for (uint32_t i = 0; i < property_count; i++) {
        // Read property name (packed string: uint32 length + bytes)
        uint32_t name_length;
        memcpy(&name_length, data_pointer, sizeof(uint32_t));
        data_pointer += sizeof(uint32_t);
        char name[256] = {0};
        memcpy(name, data_pointer, name_length);
        data_pointer += name_length;

        // Read type tag
        uint8_t type_tag = *data_pointer;
        data_pointer += 1;

        if (type_tag == 0) {
            // Float value
            float value;
            memcpy(&value, data_pointer, sizeof(float));
            data_pointer += sizeof(float);
            LOG_DEBUG("  Property: name='%s', value=%f", name, value);

            if (strcmp(name, "sge_speed") == 0) tumble_component->speed = value;
            else if (strcmp(name, "sge_direction_x") == 0) tumble_component->direction.x = value;
            else if (strcmp(name, "sge_direction_y") == 0) tumble_component->direction.y = value;
            else if (strcmp(name, "sge_direction_z") == 0) tumble_component->direction.z = value;
        } else if (type_tag == 1) {
            // String value — skip it
            uint32_t str_length;
            memcpy(&str_length, data_pointer, sizeof(uint32_t));
            data_pointer += sizeof(uint32_t) + str_length;
        }
    }
    LOG_DEBUG("Parsed tumble component on entity %d: speed=%f, direction=(%f, %f, %f)", entity, tumble_component->speed, tumble_component->direction.x, tumble_component->direction.y, tumble_component->direction.z);
}
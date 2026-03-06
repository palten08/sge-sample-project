#include "../include/components.h"
#include "types.h"
#include "ecs.h"
#include "scene.h"

int ORBIT;
int TUMBLE;

void parse_orbit_component(Scene *scene, Entity entity, int component_id, JSON_Object *json) {
    OrbitComponent *orbit_component = get_component(scene, component_id, entity);
    if (!orbit_component) {
        return; // Failed to get the component
    }
    JSON_Object *center_json = json_object_get_object(json, "center");
    if (center_json) {
        orbit_component->center.x = json_object_get_number(center_json, "x");
        orbit_component->center.y = json_object_get_number(center_json, "y");
        orbit_component->center.z = json_object_get_number(center_json, "z");
    }
    orbit_component->radius = json_object_get_number(json, "radius");
    orbit_component->speed = json_object_get_number(json, "speed");
    orbit_component->phase = json_object_get_number(json, "phase");
}

void parse_tumble_component(Scene *scene, Entity entity, int component_id, JSON_Object *json) {
    TumbleComponent *tumble_component = get_component(scene, component_id, entity);
    if (!tumble_component) {
        return; // Failed to get the component
    }
    tumble_component->speed = json_object_get_number(json, "speed");
    JSON_Object *direction_json = json_object_get_object(json, "direction");
    if (direction_json) {
        tumble_component->direction.x = json_object_get_number(direction_json, "x");
        tumble_component->direction.y = json_object_get_number(direction_json, "y");
        tumble_component->direction.z = json_object_get_number(direction_json, "z");
    }
}
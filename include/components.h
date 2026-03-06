#pragma once

#include "types.h"

extern int ORBIT;

/**
 * @brief A structure representing an orbit component that game projects can associate with an entity
 * 
 * 20 bytes
 */
typedef struct {
    Vector3f center; // 12 bytes
    float radius;   // 4 bytes
    float speed;    // 4 bytes
} OrbitComponent;

void parse_orbit_component(Scene *scene, Entity entity, int component_id, JSON_Object *json);
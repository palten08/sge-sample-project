#pragma once

#include "scene.h"
#include "app.h"

void orbit_system(Scene *scene, AppContext *app_context);
void tumble_system(Scene *scene, AppContext *app_context);
void test_camera_orbit_system(Scene *scene, AppContext *app_context);
void test_light_movement_system(Scene *scene, AppContext *app_context);
void test_camera_freelook_system(Scene *scene, AppContext *app_context);
void test_get_entity_record_system(Scene *scene, AppContext *app_context);
void test_archetype_system(Scene *scene, AppContext *app_context);
void test_add_tumble_to_entity_system(Scene *scene, AppContext *app_context);
void test_entity_system(Scene *scene, AppContext *app_context);
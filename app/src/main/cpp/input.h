//
// Created by plane on 2/13/2026.
//

#pragma once

#include "ecs.h"
#include "physics.h"

#include <map>

struct Pointer {
    vec2 pos = vec2(0.0f);
    vec2 prev_pos = vec2(0.0f);
    uint32_t down = 0;
};


struct Input_system : System {
    std::map<uint32_t, Pointer> pointers;
    vec2 prev_center;
    vec2 pivot;

    uint32_t held_pointer = 0xFFFFFFFF;
    uint32_t held_constraint;

    uint32_t player_entity = 0xFFFFFFFF;

    // inputs
    vec2 joystick = vec2(0.0f);
    uint32_t joystick_pointer = 0xFFFFFFFF;
    float jump = 0.0f;
    float jump_cooldown = 0.075f;
    float jump_charge = 0.35f;
    uint32_t jump_pointer = 0xFFFFFFFF;

    std::vector<Constraint> slime_constraints;

    Input_system();
    ~Input_system();

    void init();

    void call();
};

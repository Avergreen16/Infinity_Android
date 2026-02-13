//
// Created by plane on 2/13/2026.
//

#pragma once

#include "ecs.h"

#include <map>

struct Pointer {
    vec2 pos = vec2(0.0f);
    vec2 prev_pos = vec2(0.0f);
};


struct Input_system : System {
    std::map<uint32_t, Pointer> pointers;
    vec2 prev_center;

    Input_system();
    ~Input_system();

    void init();

    void call();
};

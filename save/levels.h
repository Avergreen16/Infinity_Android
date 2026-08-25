#pragma once

#include "ecs.h"

struct Level {
    vec4 range;
};

struct Level_system : System {
    std::vector<Level> levels;
    vec4 range;

    Level_system();

    void load_level(uint32_t i);
};
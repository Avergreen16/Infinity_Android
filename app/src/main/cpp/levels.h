#pragma once

#include "ecs.h"

struct Level {
    vec2 range;
};

struct Level_system : System {
    std::vector<Level> levels;

    Level_system();

    void load_level(uint32_t i);
};
#pragma once
#include "util.h"

struct Animation_frame {
    std::vector<ivec4> regions;
    float duration;
};

struct Animation {
    std::vector<Animation_frame> frames;
    uint32_t type = 0;
};

struct Animator {
    std::string texture;
    std::vector<Animation> animations;
    float current_animation = 0;
    uint32_t current_frame = 0;
    float current_time = 0.0f;

    vec2 size;
    vec2 rel;

    void set_animation(uint32_t id);
};
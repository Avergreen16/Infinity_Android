#pragma once
#include "wrapper.h"
#include "pnum.h"

struct Transform {
    pvec3 position;
    mat3 orientation;
};

struct Camera {
    float fov;
    mat4 proj;
};

struct Transform2D {
    vec2 position;
    mat2 orientation;
};

struct Camera2D {
    float scale;
};

#pragma once;

#include <array>
#include <vector>

#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#include "glm\glm.hpp"
#include "glm\gtx\matrix_transform_2d.hpp"
#include "glm\gtx\transform.hpp"
#include "glm\gtx\quaternion.hpp"
#include "glm\gtx\orthonormalize.hpp"

using namespace glm;

vec3 hex_color(uint32_t color);
vec3 hsv_color(float hue, float saturation, float value);
vec3 get_color(float seed);

int hash_coord(ivec3 v);
int hash_coord(ivec2 v);

struct Hash_coord {
    std::size_t operator()(const ivec3& v) const;
    std::size_t operator()(const ivec2& v) const;
};

uint32_t hash(uint32_t x);

uint32_t hash(glm::uvec2 v);

uint32_t hash(glm::uvec3 v);

uint32_t hash(glm::uvec4 v);

float to_float(uint32_t m);


uint64_t hash(uint64_t x);

uint64_t hash(vec<2, uint64_t> v);

uint64_t hash(vec<3, uint64_t> v);

uint64_t hash(vec<4, uint64_t> v);

struct Random {
    uint64_t seed;
    uint64_t value;

    Random(uint64_t init_seed);

    Random(const Random& r) = default;
    Random& operator=(const Random& r) = default;
    Random(Random&& r) = default;
    Random& operator=(Random&& r) = default;

    float operator()();
    uint64_t next();
    vec3 unit_vector();

    float operator()(glm::vec<3, uint64_t> i);
    uint64_t hash_i(glm::vec<3, uint64_t> i);
    vec3 unit_vector(glm::vec<3, uint64_t> i);
    vec3 cube_vector(glm::vec<3, uint64_t> i);
};

struct Random32 {
    uint32_t seed;
    uint32_t value;

    Random32(uint32_t init_seed);

    Random32(const Random32& r) = default;
    Random32& operator=(const Random32& r) = default;
    Random32(Random32&& r) = default;
    Random32& operator=(Random32&& r) = default;

    float operator()();
    uint32_t next();
    vec3 unit_vector();

    float operator()(uvec3 i);
    uint32_t hash_i(uvec3 i);
    vec3 unit_vector(uvec3 i);
    vec3 cube_vector(uvec3 i);
};

struct hash_uvec2 {
    std::size_t operator()(const uvec2& v) const {
        return hash(v.x ^ hash(v.y));
    }
};
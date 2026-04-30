#include "random.h"

#include <bit>
#include <thread>
#include <iostream>

constexpr int PRIME_X = 73856093;
constexpr int PRIME_Y = 19349663;
constexpr int PRIME_Z = 83492791;
constexpr float PI = 3.14159265358979f;

vec3 hex_color(uint32_t color) {
    vec3 col = vec3((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    col /= 0xFF;

    return col;
}

vec3 hsv_color(float hue, float saturation, float value) {
    vec3 color;

    hue *= 6.0f;

    float f = fract(hue);

    if(hue < 1) {
        color = vec3(1.0, f, 0.0);
    } else if(hue < 2) {
        color = vec3(1.0 - f, 1.0, 0.0);
    } else if(hue < 3) {
        color = vec3(0.0, 1.0, f);
    } else if(hue < 4) {
        color = vec3(0.0, 1.0 - f, 1.0);
    } else if(hue < 5) {
        color = vec3(f, 0.0, 1.0);
    } else if(hue < 6) {
        color = vec3(1.0, 0.0, 1.0 - f);
    }

    color = color * saturation + (1.0f - saturation);
    color *= value;

    return color;
}



std::size_t Hash_coord::operator()(const ivec3& v) const {
    int hash = v.x * PRIME_X;
    hash ^= v.y * PRIME_Y;
    hash ^= v.z * PRIME_Z;

    hash ^= (hash >> 13);
    hash = hash * 60493 + 19990303;
    return abs(hash) % 16;
}

std::size_t Hash_coord::operator()(const ivec2& v) const {
    int hash = v.x * PRIME_X;
    hash ^= v.y * PRIME_Y;

    hash ^= (hash >> 13);
    hash = hash * 60493 + 19990303;
    return abs(hash) % 16;
}

uint32_t hash(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

uint32_t hash(glm::uvec2 v) {
    return hash(v.x ^ hash(v.y));
}

uint32_t hash(glm::uvec3 v) {
    return hash(v.x ^ hash(v.y)) ^ hash(v.z);
}

uint32_t hash(glm::uvec4 v) {
    return hash(v.x ^ hash(v.y)) ^ hash(v.z) ^ hash(v.w);
}

float to_float(uint32_t m) {
    const uint32_t ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint32_t ieeeOne = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float f;
    std::memcpy(&f, &m, 4); // Range [1:2]
    return f * 2.0f - 3.0f;                // Range [-1:1]
}

float to_float_10(uint32_t m) {
    const uint32_t ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint32_t ieeeOne = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float f;
    std::memcpy(&f, &m, 4); // Range [1:2]


    return f - 1.0f;                // Range [0, 1]
}

uint64_t hash(uint64_t x) {
    x += 1;
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccd;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53;
    x ^= x >> 33;

    return x;
}

uint64_t hash(vec<2, uint64_t> v) {
    return hash(v.x ^ hash(v.y));
}

uint64_t hash(vec<3, uint64_t> v) {
    return hash(v.x ^ hash(v.y)) ^ hash(v.z);
}

uint64_t hash(vec<4, uint64_t> v) {
    return hash(v.x ^ hash(v.y)) ^ hash(v.z) ^ hash(v.w);
}

Random32::Random32(uint32_t init_seed) {
    seed = init_seed;
    value = seed;
}

int hash_coord(ivec3 v, uint32_t seed) {
    int h = v.x * PRIME_X;
    h ^= v.y * PRIME_Y;
    h ^= v.z * PRIME_Z;

    h ^= (h >> 13);
    h = h * 60493 + 19990303;
    h ^= hash(seed);
    return abs(h) % 16;
}

int hash_coord(ivec2 v, uint32_t seed) {
    int h = v.x * PRIME_X;
    h ^= v.y * PRIME_Y;

    h ^= (h >> 13);
    h = h * 60493 + 19990303;
    h ^= hash(seed);
    return abs(h) % 16;
}


vec2 get_vector_v2(ivec2 v, uint32_t seed) {
    int h = v.x * PRIME_X;
    h ^= v.y * PRIME_Y;

    h ^= (h >> 13);
    h = h * 60493 + 19990303;
    h ^= hash(seed);
    float f = abs(float(h) / 0x7FFFFFFF) * 2.0f * PI;

    return vec2(cos(f), sin(f));
}

vec3 get_vector_v3(ivec3 v, uint32_t seed) {
    int h = 0.0f;

    while(true) {
        h ^= v.x * PRIME_X;
        h ^= v.y * PRIME_Y;
        h ^= v.z * PRIME_Z;

        h ^= (h >> 13);
        h = h * 60493 + 19990303;
        h ^= hash(seed);

        uint32_t hh;
        std::memcpy(&hh, &h, 4);
        uint32_t hx = hh & 0x3FF;
        uint32_t hy = (hh >> 10) & 0x3FF;
        uint32_t hz = (hh >> 20) & 0x3FF;

        vec3 vector = vec3(hx, hy, hz) / float(0x3FF);
        vector = vector * 2.0f - 1.0f;
        float len = length(vector);

        if(len < 1.0f) {
            vector /= len;
            return vector;
        }
    }
}

float Random32::operator()() {
    value = hash(value);

    return to_float(value);
}

uint32_t Random32::next() {
    value = hash(value);

    return value;
}

vec3 Random32::unit_vector() {
    vec3 r;

    bool c = true;
    while(c) {
        r = {operator()(), operator()(), operator()()};

        float len = length(r);
        if(!(len > 1 || len == 0)) c = false;
    }

    r = normalize(r);

    return r;
}


float Random32::operator()(uvec3 i) {
    uint32_t hash_value = hash(uvec4(i, seed));

    return to_float(hash_value);
}

uint32_t Random32::hash_i(uvec3 i) {
    uint32_t hash_value = hash(uvec4(i, seed));

    return hash_value;
}

vec3 Random32::unit_vector(uvec3 i) {
    uint32_t current_value = seed;

    glm::vec3 unit_vector;
    float u_length = 10;
    while(u_length > 1.0 || u_length == 0.0) {
        current_value = hash(current_value);
        float x = to_float(hash(i) ^ current_value);
        current_value = hash(current_value);
        float y = to_float(hash(i) ^ current_value);
        current_value = hash(current_value);
        float z = to_float(hash(i) ^ current_value);

        unit_vector = glm::vec3(x, y, z);
        u_length = glm::length(unit_vector);
    }

    return normalize(unit_vector);
}

vec3 Random32::cube_vector(uvec3 i) {
    uint32_t current_value = seed;

    glm::vec3 vector;
    current_value = hash(current_value);
    float x = to_float(hash(i) ^ current_value);
    current_value = hash(current_value);
    float y = to_float(hash(i) ^ current_value);
    current_value = hash(current_value);
    float z = to_float(hash(i) ^ current_value);

    vector = {x, y, z};

    return vector;
}

Random::Random(uint64_t init_seed) {
    seed = init_seed;
    value = seed;
}

float Random::operator()() {
    value = hash(value);

    return to_float(value);
}

uint64_t Random::next() {
    value = hash(value);

    return value;
}


vec3 Random::unit_vector() {
    vec3 r;

    bool c = true;
    while(c) {
        r = {operator()(), operator()(), operator()()};

        float len = length(r);
        if(!(len > 1 || len == 0)) c = false;
    }

    r = normalize(r);

    return r;
}

float Random::operator()(glm::vec<3, uint64_t> i) {
    uint32_t hash_value = hash(uvec4(i, seed));

    return to_float(hash_value);
}

uint64_t Random::hash_i(glm::vec<3, uint64_t> i) {
    uint64_t hash_value = hash(uvec4(i, seed));

    return hash_value;
}

vec3 Random::unit_vector(glm::vec<3, uint64_t> i) {
    uint64_t current_value = seed;

    glm::vec3 unit_vector;
    float u_length = 10;
    while(u_length > 1.0 || u_length == 0.0) {
        current_value = hash(current_value);
        float x = to_float(hash(i) ^ current_value);
        current_value = hash(current_value);
        float y = to_float(hash(i) ^ current_value);
        current_value = hash(current_value);
        float z = to_float(hash(i) ^ current_value);

        unit_vector = glm::vec3(x, y, z);
        u_length = glm::length(unit_vector);
    }

    return normalize(unit_vector);
}

vec3 Random::cube_vector(glm::vec<3, uint64_t> i) {
    uint64_t current_value = seed;

    glm::vec3 vector;
    current_value = hash(current_value);
    float x = to_float(hash(i) ^ current_value);
    current_value = hash(current_value);
    float y = to_float(hash(i) ^ current_value);
    current_value = hash(current_value);
    float z = to_float(hash(i) ^ current_value);

    vector = {x, y, z};

    return vector;
}
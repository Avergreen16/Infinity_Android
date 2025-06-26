#pragma once
#include <shared_mutex>
#include <queue>
#include <vector>
#include <functional>
#include <optional>
#include <chrono>
#include <thread>
#include <iostream>
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#include "glm\glm.hpp"
#include "glm\gtx\matrix_transform_2d.hpp"
#include "glm\gtx\transform.hpp"
#include "glm\gtx\quaternion.hpp"

using namespace glm;

mat3 rotate_to(vec3 a, vec3 b);

double get_time();

time_t get_time_t();

struct Thread_pool {
    std::queue<std::unique_ptr<std::function<void()>>> tasks;
    std::shared_mutex task_mutex;
    std::vector<std::thread> threads;

    bool stop = false;

    Thread_pool(int num_threads);

    ~Thread_pool();

    void add_task(std::function<void()> f);
};

struct Time {
    std::chrono::steady_clock::time_point last_time;

    void overwrite();

    Time();

    double get_elapsed_time(bool overwrite = false);
};

struct Profiler {
    std::vector<double> times;
    std::vector<std::string> names;
    uint32_t current_pos = 0;
    double prev_time;
    uint32_t iterations = 0;

    void restart() {
        times.clear();
        current_pos = 0;
        prev_time = get_time();

        iterations = 0;
    }

    void reset() {
        current_pos = 0;
        prev_time = get_time();

        ++iterations;
    }

    void step(std::string name = "") {
        double current_time = get_time();
        double diff = current_time - prev_time;
        prev_time = current_time;

        std::string new_name = std::to_string(current_pos);
        if(name.size()) {
            new_name = name;
        }

        if(times.size() <= current_pos) {
            times.push_back(diff);
            names.push_back(name);
        } else {
            times[current_pos] += diff;
            names[current_pos] = name;
        }

        //std::cout << name << "\n";

        ++current_pos;
    }   

    void output() {
        double total = 0;
        for(double t : times) total += t;

        double total_time = 0.0f;

        uint32_t number = 0;
        for(double t : times) {
            total_time += t;

            std::cout << names[number] << " : " << t / iterations << " = " << (t / total) * 100 << "%\n";
            ++number;
        }
        
        std::cout << total_time / iterations << "\n";

        std::cout << "\n";
    }
};

struct Texture;

std::shared_ptr<Texture> create_image_texture(std::string path, ivec4 range, vec3 color);

extern Profiler profiler;
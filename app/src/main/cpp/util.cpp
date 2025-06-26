#include "util.h"
#include "wrapper.h"

using namespace std::chrono;

mat3 rotate_to(vec3 a, vec3 b) {
    vec3 cross_p = cross(a, b);
    float d = dot(a, b);
    float f = acos(d);
    float len_cross = length(cross_p);
    vec3 n = cross_p / len_cross;

    if(isinf(n.x) || isnan(n.x) || f == 0 || isnan(f)) {
        mat3 rot_mat = identity<mat3>();
        if(dot(a, b) < 0) rot_mat = mat3(rot_mat[0], -rot_mat[1], -rot_mat[2]);
        return rot_mat;
    }

    return rotate(f, n);
}

double get_time() {
    return (double)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() / 1000000;
}

time_t get_time_t() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

Thread_pool::Thread_pool(int num_threads) {
    for(int i = 0; i < num_threads; ++i) {
        threads.emplace_back(std::thread([this, i]() {
            while(!stop) {
                std::function<void()> task;

                while(!tasks.size()) {
                    if(stop) goto stop_o;
                    std::this_thread::yield();
                }

                task_mutex.lock_shared();
                if(tasks.size()) {
                    task_mutex.unlock_shared();
                    task_mutex.lock();
                    if(tasks.size()) {
                        task = std::move(*tasks.front().get());
                        tasks.pop();
                        task_mutex.unlock();

                        task();
                    } else task_mutex.unlock();
                } else task_mutex.unlock_shared();

                stop_o:
            }
        }));
    }
}

Thread_pool::~Thread_pool() {
    stop = true;

    for(std::thread& t : threads) {
        t.join();
    }
}

void Thread_pool::add_task(std::function<void()> f) {
    task_mutex.lock();
    tasks.emplace(std::make_unique<std::function<void()>>(std::move(f)));
    task_mutex.unlock();
}

void Time::overwrite() {
    last_time = steady_clock::now();
}

Time::Time() {
    overwrite();
}

double Time::get_elapsed_time(bool overwrite) {
    steady_clock::time_point current_time = steady_clock::now();
    steady_clock::duration duration = current_time - last_time;

    if(overwrite) {
        last_time = current_time;
    }

    return double(duration.count()) * steady_clock::period::num / steady_clock::period::den;
}

std::shared_ptr<Texture> create_image_texture(std::string path, ivec4 range, vec3 color) {
    ivec3 size;

    ivec2 isize;
    std::vector<uint8_t> bytes = get_texture(path, isize);

    size = ivec3(isize, 4);

    auto retrieve = [](std::vector<uint8_t>& data, ivec3 size, ivec4 range) {
        std::vector<uint8_t> return_data;
        return_data.resize(range.z * range.w * size.z);

        for(int y = 0; y < range.w; ++y) {
            for(int x = 0; x < range.z; ++x) {
                uvec2 src_index = uvec2(range.x + x, range.y + y);
                uint32_t src_pos = src_index.y * size.x + src_index.x;
                uint32_t dst_pos = y * range.z + x;

                for(int i = 0; i < 4; ++i) {
                    return_data[dst_pos * 4 + i] = data[src_pos * 4 + i];
                }
            }
        }

        return return_data;
    };

    ivec2 output_size = range.zw();
    std::vector<uint8_t> output = retrieve(bytes, size, range);

    for(int i = 0; i < output.size() / 4; ++i) {
        vec3 output_color = {output[i * 4], output[i * 4 + 1], output[i * 4 + 2]};
        output_color = (output_color / 255.0f * color) * 255.0f;

        output[i * 4] = output_color[0];
        output[i * 4 + 1] = output_color[1];
        output[i * 4 + 2] = output_color[2];
    }

    std::shared_ptr<Texture> t(new Texture(output.data(), uvec3(output_size.x, output_size.y, 1), GL_TEXTURE_2D, {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}));

    return t;
}


Profiler profiler;
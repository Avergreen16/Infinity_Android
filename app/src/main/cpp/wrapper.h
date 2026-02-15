#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <cstdint>

#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <GLES3/gl3.h>
#include <EGL/egl.h>

using namespace glm;

std::string get_text_from_file(std::string path);

std::vector<uint8_t> get_bytes_from_file(std::string path);

std::vector<uint8_t> get_texture(std::string path, ivec2& size);

struct Shader {
    uint32_t id;

    Shader() = default;
    Shader(std::string vspath, std::string fspath);

    void from_source(std::string vspath, std::string fspath);
    void use();
};

struct Vertices {
    uint32_t vertex_buffer;
    uint32_t vertex_array;
    uint32_t index_buffer;

    uint32_t num_vertices;
    uint32_t num_indices;
    EGLenum index_type;

    bool initialized = false;

    Vertices() = default;

    Vertices(const Vertices& a) = delete;

    Vertices& operator=(const Vertices& a) = delete;

    Vertices(Vertices&& a);

    ~Vertices();

    Vertices& operator=(Vertices&& a);

    void init();

    void vertex_buffer_data(void* ptr, uint32_t num_vertices_, uint32_t vertex_size, uint32_t usage);

    void index_buffer_data(void* ptr, uint32_t num_indices_, GLenum index_type_, uint32_t index_size, uint32_t usage);

    void add_vertex_attribute(uint32_t index, uint32_t size, GLenum type, GLenum normalized, uint32_t stride, uint32_t offset);

    void bind();

    void draw_vertices(GLenum mode);

    void draw_indices(GLenum mode);
};


struct Format {
    GLenum format_bits;
    GLenum format;
    GLenum bits;
};

struct Texture {
    GLuint id;
    GLenum type;
    glm::ivec3 size;
    int num_channels;
    Format format;

    Texture(std::string path, Format format, int mip_levels = 0);

    Texture(uint8_t* data, glm::uvec3 size, GLenum type, Format format);

    Texture(glm::uvec3 size, GLenum type, Format format);

    bool load(std::string path, Format format, int mip_levels = 0);

    bool load(glm::uvec2 size, Format format);

    bool load(glm::uvec3 size, Format format);

    void bind(int binding);

    void bind();

    Texture() = default;

    Texture(const Texture& t) = delete;

    Texture(Texture&& t) noexcept;

    Texture& operator=(Texture&& t) noexcept;

    void delete_texture();

    ~Texture();
};

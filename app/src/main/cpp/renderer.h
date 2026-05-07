//
// Created by plane on 6/24/2025.
//

#pragma once

#include "ecs.h"
#include "wrapper.h"
#include "physics.h"

#include "GLES/egl.h"
#include <game-activity/native_app_glue/android_native_app_glue.h>

struct Mesh {
    std::vector<vec2> v;
    std::vector<uint32_t> i;

    vec3 color = vec3(0.35f);

    std::shared_ptr<Vertices> v_lines;
    std::shared_ptr<Vertices> v_tris;
};

struct Sprite {
    vec3 color = vec3(0.35f);
    float border = 0.0f;

    std::shared_ptr<Texture> color_tex;
    std::shared_ptr<Texture> normal_tex;

    vec4 range;
    vec2 size;
    vec2 offset;
};

struct Texture_vertex {
    vec3 pos;
    vec2 tex;
};

void create_mesh(Mesh& m, std::vector<vec2> v, vec2 radius);
void create_mesh(Mesh& m, Collider& collider);

struct Renderer : System {
    Renderer();
    ~Renderer();

    bool render = false;

    android_app* app;
    EGLDisplay display;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    std::shared_ptr<Vertices> vertices;
    std::vector<std::shared_ptr<Framebuffer>> framebuffers;
    uint32_t camera;

    std::vector<vec2> points;
    std::vector<vec2> normals;
    std::vector<vec3> cs;

    void init();

    void call();

    void terminate();

    void render_background();

    void render_gui();

    void render_mesh(uint32_t entity);

    void render_sprite(uint32_t entity);

    void render_buttons();

    void render_points();
};
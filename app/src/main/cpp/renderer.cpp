//
// Created by plane on 6/24/2025.
//

#include "renderer.h"
#include "logging.h"
#include "wrapper.h"
#include "core.h"
#include "camera.h"
#include "input.h"
#include "gui.h"

#include <GLES/egl.h>

using namespace glm;

void Renderer::init() {
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    auto res = eglInitialize(display, nullptr, nullptr);
    {
        EGLint attribs[] = {
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_NONE
        };
        EGLint num_configs;

        eglChooseConfig(display, attribs, &config, 1, &num_configs);
    }

    surface = eglCreateWindowSurface(display, config, app->window, nullptr);

    {
        EGLint attribs[] = {
                EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE
        };

        context = eglCreateContext(display, config, nullptr, attribs);
    }

    eglMakeCurrent(display, surface, surface, context);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

Renderer::Renderer() {
    Signature s = ecs.update_signature<Transform>();
    collectors.push_back(Collector{s, false});

    s = ecs.update_signature<Transform>();
    ecs.update_signature<Camera>(s);
    collectors.push_back(Collector{s, false});
}

void Renderer::terminate() {
    eglDestroyContext(display, context);
    eglDestroySurface(display, surface);
    eglTerminate(display);
}

Renderer::~Renderer() {
    terminate();
}

void Renderer::call() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);
    glEnable(GL_BLEND);

    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE);
    glClearDepthf(0.0f);
    //glDepthRangef(1.0f, 0.0f);

    int width, height;
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);

    for(uint32_t entity : collectors[1].entities) {
        Transform& t = ecs.get_component<Transform>(entity);
        Camera& c = ecs.get_component<Camera>(entity);
        c.proj = core.get_proj_matrix(ivec2(width, height), c.fov, 0.0625, 256.0f, 1.0f, 0.0f);
    }

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(!vertices) {
        vertices = std::shared_ptr<Vertices>(new Vertices);
        vertices->init();
    }

    glDisable(GL_DEPTH_TEST);

    render_background();

    //render_text();

    render_gui();

    eglSwapBuffers(display, surface);
}

void Renderer::render_background() {
    Input_system& input_system = ecs.get_system<Input_system>();
    std::vector<vec2> square = {
            vec2(-1.0f, -1.0f),
            vec2(1.0f, -1.0f),
            vec2(1.0f, 1.0f),
            vec2(-1.0f, -1.0f),
            vec2(1.0f, 1.0f),
            vec2(-1.0f, 1.0f)
    };

    vertices->vertex_buffer_data(square.data(), square.size(), sizeof(vec2), GL_STREAM_DRAW);
    vertices->add_vertex_attribute(0, 2, GL_FLOAT, false, sizeof(vec2), 0);

    Transform2D& ct = ecs.get_component<Transform2D>(camera);
    Camera2D& cc = ecs.get_component<Camera2D>(camera);

    mat4 inv_rot = mat4(transpose(ct.orientation));

    mat4 view = inv_rot * scale(vec3(cc.scale, cc.scale, 1.0f)) * translate(vec3(-ct.position, 0.0f));

    int width, height;
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);

    float aspect_ratio = float(height) / width;
    mat4 proj = scale(vec3(1.0f, 1.0f / aspect_ratio, 1.0f));

    core.shaders["background"]->use();

    vertices->bind();

    glUniformMatrix4fv(0, 1, false, &view[0][0]);
    glUniformMatrix4fv(1, 1, false, &proj[0][0]);
    glUniform2f(2, float(width), float(height));

    vertices->draw_vertices(GL_TRIANGLES);
}

void Renderer::render_gui() {
    GUI_system& gui = ecs.get_system<GUI_system>();
    Transform2D& ct = ecs.get_component<Transform2D>(camera);
    Camera2D& cc = ecs.get_component<Camera2D>(camera);

    int width, height;
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);

    float text_scale = 3.0f;

    mat4 proj = translate(vec3(-1.0f, -1.0f, 0.0f)) * scale(vec3(2.0f / float(width), 2.0f / float(height), 1.0f));
    mat4 view = identity<mat4>();
    mat4 model = identity<mat4>();

    std::vector<UI_vertex>& vs = gui.vertices;

    vertices->vertex_buffer_data(vs.data(), vs.size(), sizeof(UI_vertex), GL_STREAM_DRAW);
    vertices->add_vertex_attribute(0, 2, GL_FLOAT, false, sizeof(UI_vertex), 0);
    vertices->add_vertex_attribute(1, 2, GL_FLOAT, false, sizeof(UI_vertex), sizeof(float) * 2);
    vertices->add_vertex_attribute(2, 4, GL_FLOAT, false, sizeof(UI_vertex), sizeof(float) * 4);
    vertices->add_vertex_attribute(3, 4, GL_FLOAT, false, sizeof(UI_vertex), sizeof(float) * 8);
    vertices->add_vertex_attribute(4, 1, GL_UNSIGNED_INT, false, sizeof(UI_vertex), sizeof(float) * 12);

    core.shaders["gui"]->use();

    vertices->bind();

    glUniformMatrix4fv(0, 1, false, &proj[0][0]);
    glUniformMatrix4fv(1, 1, false, &view[0][0]);
    glUniformMatrix4fv(2, 1, false, &model[0][0]);

    core.textures["text"]->bind(0);
    core.textures["gui"]->bind(1);

    vertices->draw_vertices(GL_TRIANGLES);
}
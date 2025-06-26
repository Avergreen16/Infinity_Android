//
// Created by plane on 6/24/2025.
//

#pragma once

#include "ecs.h"
#include "wrapper.h"

#include "GLES/egl.h"
#include <game-activity/native_app_glue/android_native_app_glue.h>

struct Renderer : System {
    Renderer();
    ~Renderer();

    android_app* app;
    EGLDisplay display;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    std::shared_ptr<Vertices> vertices;
    uint32_t camera;

    void init();

    void call();

    void terminate();
};
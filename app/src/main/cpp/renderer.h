//
// Created by plane on 6/24/2025.
//

#pragma once

#include "GLES/egl.h"
#include <game-activity/native_app_glue/android_native_app_glue.h>

struct Renderer {
    Renderer(android_app* app);
    ~Renderer();

    EGLDisplay display;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    void do_frame();
};
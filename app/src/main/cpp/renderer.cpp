//
// Created by plane on 6/24/2025.
//

#include "renderer.h"
#include "logging.h"
#include <GLES/egl.h>


Renderer::Renderer(android_app* app) {
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    auto res = eglInitialize(display, nullptr, nullptr);
    {
        EGLint attribs[] = {
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
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

    LOGD("renderer initialized");

    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
}

Renderer::~Renderer() {
    eglDestroyContext(display, context);
    eglDestroySurface(display, surface);
    eglTerminate(display);
}

void Renderer::do_frame() {
    int width, height;
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(display, surface);
}
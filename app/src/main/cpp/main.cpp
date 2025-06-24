#include <jni.h>

#include "AndroidOut.h"
#include "logging.h"
#include "renderer.h"

#include <game-activity/GameActivity.cpp>
#include <game-text-input/gametextinput.cpp>

extern "C" {
#include <game-activity/native_app_glue/android_native_app_glue.c>
void handle_cmd(android_app *app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            // create window
            LOGD("init game");
            app->userData = new Renderer(app);
            break;
        case APP_CMD_TERM_WINDOW:
            // destroy window
            if (app->userData) {
                Renderer* renderer = (Renderer*)app->userData;
                if(renderer) delete renderer;
            }
            break;
        default:
            break;
    }
}


bool motion_event_filter_func(const GameActivityMotionEvent *motionEvent) {
    auto sourceClass = motionEvent->source & AINPUT_SOURCE_CLASS_MASK;
    return (sourceClass == AINPUT_SOURCE_CLASS_POINTER ||
            sourceClass == AINPUT_SOURCE_CLASS_JOYSTICK);
}

void android_main(android_app* app) {
    app->onAppCmd = handle_cmd;

    android_poll_source* poll_source;
    int events;
    do {
        if(ALooper_pollOnce(0, nullptr, &events, (void**)&poll_source) >= 0) {
            if(poll_source) poll_source->process(app, poll_source);
        }

        if(!app->userData) continue;

        Renderer* renderer = (Renderer*)app->userData;

        renderer->do_frame();

    } while(!app->destroyRequested);
}
}
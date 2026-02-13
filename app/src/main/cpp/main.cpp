#include <jni.h>

#include "AndroidOut.h"
#include "logging.h"
#include "renderer.h"
#include "core.h"
#include "camera.h"
#include "ecs.h"

#include <game-activity/GameActivity.cpp>
#include <game-text-input/gametextinput.cpp>

extern "C" {
#include <game-activity/native_app_glue/android_native_app_glue.c>
void handle_cmd(android_app *app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW: {
            // create window
            LOGD("init game");

            Renderer &renderer = ecs.get_system<Renderer>();
            renderer.app = app;
            renderer.init();

            app->userData = &renderer;

            core.shaders.clear();
            core.textures.clear();

            core.shaders.emplace("color", std::shared_ptr<Shader>(new Shader("shaders/color.vert", "shaders/color.frag")));
            core.shaders.emplace("grid", std::shared_ptr<Shader>(new Shader("shaders/grid.vert", "shaders/grid.frag")));
            core.shaders.emplace("background", std::shared_ptr<Shader>(new Shader("shaders/background.vert", "shaders/background.frag")));
            core.textures.emplace("grid", std::shared_ptr<Texture>(new Texture("textures/grid.png", {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}, 4)));
            break;
        }
        case APP_CMD_TERM_WINDOW: {
            // destroy window
            Renderer &renderer = ecs.get_system<Renderer>();
            //renderer.~Renderer();
            break;
        }
        default: {
            break;
        }
    }
}


bool motion_event_filter_func(const GameActivityMotionEvent *motionEvent) {
    auto sourceClass = motionEvent->source & AINPUT_SOURCE_CLASS_MASK;
    return (sourceClass == AINPUT_SOURCE_CLASS_POINTER ||
            sourceClass == AINPUT_SOURCE_CLASS_JOYSTICK);
}

void android_main(android_app* app) {
    ecs.entity_manager.init();

    ecs.register_system<Renderer>();

    ecs.register_component<Transform>();
    ecs.register_component<Camera>();

    app->onAppCmd = handle_cmd;

    Camera camera;
    Transform transform;
    camera.fov = 90.0f;
    transform.position = vec3(0.0f, 1.0f, 1.0f);
    transform.orientation = create_rot_mat(vec3(0, 0, 1), normalize(vec3(0.0f, 1.0f, 1.0f)));

    uint32_t entity = ecs.insert_entity();
    ecs.insert_component(entity, transform);
    ecs.insert_component(entity, camera);

    Renderer& renderer = ecs.get_system<Renderer>();
    renderer.camera = entity;

    android_poll_source* poll_source;

    double time = get_time();
    core.start_time = time;

    int events;
    do {
        if(ALooper_pollOnce(0, nullptr, &events, (void**)&poll_source) >= 0) {
            if(poll_source) poll_source->process(app, poll_source);
        }
        if(!app->userData) continue;


        double new_time = get_time();
        double diff_time = new_time - time;

        time = new_time;
        core.current_time = time;
        core.delta_time = diff_time;

        for(std::string& name : ecs.system_manager.call_order) {
            //std::cout << name;
            auto& system = ecs.system_manager.systems[name];
            system->call();
        }
    } while(!app->destroyRequested);
}
}
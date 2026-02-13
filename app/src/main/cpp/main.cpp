#include <jni.h>

#include "AndroidOut.h"
#include "logging.h"
#include "renderer.h"
#include "core.h"
#include "camera.h"
#include "ecs.h"
#include "input.h"

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
    ecs.register_system<Input_system>();

    ecs.register_component<Transform>();
    ecs.register_component<Camera>();
    ecs.register_component<Transform2D>();
    ecs.register_component<Camera2D>();

    app->onAppCmd = handle_cmd;

    Camera2D camera;
    Transform2D transform;
    camera.scale = 1.0f;
    transform.position = vec2(0.0f);
    transform.orientation = identity<mat2>();

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

        Input_system& input_system = ecs.get_system<Input_system>();

        bool done = false;
        do {
            int result = ALooper_pollOnce(0, nullptr, &events, (void**)&poll_source);


            switch(result) {
                case ALOOPER_POLL_WAKE:
                case ALOOPER_POLL_TIMEOUT:
                    done = true;
                    break;

                case ALOOPER_POLL_ERROR:
                    LOGD("ALooper_pollOnce returned an ERROR");
                    break;

                case ALOOPER_POLL_CALLBACK:
                    break;

                default:
                    if(poll_source) poll_source->process(app, poll_source);
            }
        } while(!done);

        android_input_buffer* input_buffer = android_app_swap_input_buffers(app);
        if(input_buffer) {
            for(std::size_t i = 0; i < input_buffer->motionEventsCount; ++i) {
                GameActivityMotionEvent event = input_buffer->motionEvents[i];

                if(event.pointerCount > 0) {
                    uint32_t masked_action = event.action & AMOTION_EVENT_ACTION_MASK;
                    uint32_t pointer_index = (event.action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

                    if(masked_action == AMOTION_EVENT_ACTION_POINTER_DOWN || masked_action == AMOTION_EVENT_ACTION_DOWN) {
                        float x = GameActivityPointerAxes_getX(&event.pointers[pointer_index]);
                        float y = GameActivityPointerAxes_getY(&event.pointers[pointer_index]);

                        uint32_t id = event.pointers[pointer_index].id;
                        LOGD("%i DOWN", id);

                        Pointer p;
                        p.pos = {x, y};
                        p.prev_pos = p.pos;

                        input_system.pointers.emplace(id, p);

                        /*

                        int width, height;
                        eglQuerySurface(renderer.display, renderer.surface, EGL_WIDTH, &width);
                        eglQuerySurface(renderer.display, renderer.surface, EGL_HEIGHT, &height);

                        Camera2D& c_camera = ecs.get_component<Camera2D>(entity);
                        Transform2D& c_transform = ecs.get_component<Transform2D>(entity);

                        vec2 pos = vec2(1.0 - x, y);
                        float ratio = width / c_camera.scale * 0.5f;
                        pos /= ratio;

                        c_transform.position = pos;
                        */
                    } else if(masked_action == AMOTION_EVENT_ACTION_MOVE) {
                        for(int j = 0; j < event.pointerCount; ++j) {
                            float x = GameActivityPointerAxes_getX(&event.pointers[j]);
                            float y = GameActivityPointerAxes_getY(&event.pointers[j]);

                            vec2 new_pos = vec2(x, y);

                            uint32_t id = event.pointers[j].id;

                            Pointer& p = input_system.pointers[id];

                            p.pos = new_pos;
                        }

                        /*
                        float x = GameActivityPointerAxes_getX(&event.pointers[0]);
                        float y = GameActivityPointerAxes_getY(&event.pointers[0]);

                        int width, height;
                        eglQuerySurface(renderer.display, renderer.surface, EGL_WIDTH, &width);
                        eglQuerySurface(renderer.display, renderer.surface, EGL_HEIGHT, &height);

                        Camera2D& c_camera = ecs.get_component<Camera2D>(entity);
                        Transform2D& c_transform = ecs.get_component<Transform2D>(entity);

                        vec2 pos = vec2(1.0 - x, y);
                        float ratio = width / c_camera.scale * 0.5f;
                        pos /= ratio;

                        c_transform.position = pos;
                        */
                    } else if(masked_action == AMOTION_EVENT_ACTION_POINTER_UP || masked_action == AMOTION_EVENT_ACTION_UP) {
                        uint32_t id = event.pointers[pointer_index].id;
                        LOGD("%i UP", id);

                        input_system.pointers.erase(id);
                    }
                }
            }
            android_app_clear_motion_events(input_buffer);
        }


        if(!app->userData) continue;

        /*
        if(ALooper_pollOnce(0, nullptr, &events, (void**)&poll_source) >= 0) {
            if(poll_source) poll_source->process(app, poll_source);
        }
        if(!app->userData) continue;
        */


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
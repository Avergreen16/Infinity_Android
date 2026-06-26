#include <jni.h>

#include "AndroidOut.h"
#include "logging.h"
#include "renderer.h"
#include "core.h"
#include "camera.h"
#include "ecs.h"
#include "input.h"
#include "gui.h"
#include "physics.h"
#include "levels.h"

#include <game-activity/GameActivity.cpp>
#include <game-text-input/gametextinput.cpp>

extern "C" {
#include <game-activity/native_app_glue/android_native_app_glue.c>
void handle_cmd(android_app *app, int32_t cmd) {
    switch(cmd) {
        case APP_CMD_INIT_WINDOW: {
            // create window

            Renderer& renderer = ecs.get_system<Renderer>();
            renderer.app = app;
            renderer.init();

            renderer.render = true;
            app->userData = &renderer;

            GUI_system& gui_system = ecs.get_system<GUI_system>();
            gui_system.init();

            core.shaders.emplace("color", std::shared_ptr<Shader>(new Shader("shaders/color.vert", "shaders/color.frag")));
            core.shaders.emplace("screen", std::shared_ptr<Shader>(new Shader("shaders/screen.vert", "shaders/screen.frag")));
            core.shaders.emplace("background", std::shared_ptr<Shader>(new Shader("shaders/background.vert", "shaders/background.frag")));
            core.shaders.emplace("gui", std::shared_ptr<Shader>(new Shader("shaders/gui.vert", "shaders/gui.frag")));
            core.shaders.emplace("shape", std::shared_ptr<Shader>(new Shader("shaders/shape.vert", "shaders/shape.frag")));
            core.shaders.emplace("shape_render", std::shared_ptr<Shader>(new Shader("shaders/shape_render.vert", "shaders/shape_render.frag")));
            core.shaders.emplace("sprite", std::shared_ptr<Shader>(new Shader("shaders/sprite.vert", "shaders/sprite.frag")));
            core.shaders.emplace("slime", std::shared_ptr<Shader>(new Shader("shaders/slime.vert", "shaders/slime.frag")));
            core.shaders.emplace("texture", std::shared_ptr<Shader>(new Shader("shaders/texture.vert", "shaders/texture.frag")));

            core.textures.emplace("grid", std::shared_ptr<Texture>(new Texture("textures/grid.png", {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}, 4)));
            core.textures.emplace("text", std::shared_ptr<Texture>(new Texture("textures/text_mono.png", {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}, 4)));
            core.textures.emplace("gui", std::shared_ptr<Texture>(new Texture("textures/icons.png", {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}, 4)));
            core.textures.emplace("slime", std::shared_ptr<Texture>(new Texture("textures/slime.png", {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}, 4)));
            core.textures.emplace("robot_small", std::shared_ptr<Texture>(new Texture("textures/robot_small.png", {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}, 4)));
            core.textures.emplace("robot_big", std::shared_ptr<Texture>(new Texture("textures/robot_big.png", {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}, 4)));

            /*
             * when the context got reset, all of the vertex buffers got turned off...
             * but when i delete them after creating new buffers,
             * some of the old buffers have the same id as the new buffers,
             * and then deleting them deletes the new buffer
             */
            Physics_system& physics_system = ecs.get_system<Physics_system>();
            renderer.vertices.reset();

            for(uint32_t entity : physics_system.collectors[0].entities) {
                Collider& collider = ecs.get_component<Collider>(entity);
                Mesh& mesh = ecs.get_component<Mesh>(entity);

                create_mesh(mesh, collider);
                //create_sprite(sprite, collider.vertices, collider.radius, sprite.border);
            }

            break;
        }
        case APP_CMD_GAINED_FOCUS: {

            break;
        }
        case APP_CMD_LOST_FOCUS: {

            break;
        }
        case APP_CMD_TERM_WINDOW: {
            GUI_system& gui_system = ecs.get_system<GUI_system>();
            Renderer& renderer = ecs.get_system<Renderer>();
            // destroy window
            renderer.render = false;
            core.shaders.clear();
            core.textures.clear();
            renderer.terminate();
            break;
        }
        case APP_CMD_DESTROY: {
            Renderer &renderer = ecs.get_system<Renderer>();

            core.shaders.clear();
            core.textures.clear();
            renderer.terminate();
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

bool init = false;

void android_main(android_app* app) {
    if(!init) {
        ecs.entity_manager.init();

        ecs.register_system<Renderer>();
        ecs.register_system<GUI_system>();
        ecs.register_system<Input_system>();
        ecs.register_system<Physics_system>();
        ecs.register_system<Level_system>();


        Camera2D camera;
        Transform2D transform;
        camera.scale = 1.0f;
        transform.position = vec2(0.0f);
        transform.orientation = identity<mat2>();

        uint32_t entity = ecs.insert_entity();
        ecs.insert_component(entity, transform);
        ecs.insert_component(entity, camera);

        Renderer &renderer = ecs.get_system<Renderer>();
        renderer.camera = entity;

        ecs.get_system<Level_system>().load_level(0);
    }

    double time = get_time();
    core.start_time = time;

    Input_system &input_system = ecs.get_system<Input_system>();
    Physics_system &physics_system = ecs.get_system<Physics_system>();

    Renderer &renderer = ecs.get_system<Renderer>();
    app->onAppCmd = handle_cmd;
    android_poll_source *poll_source;

    LOGD("MAIN CALLED");

    int events;
    do {
        int width, height;
        eglQuerySurface(renderer.display, renderer.surface, EGL_WIDTH, &width);
        eglQuerySurface(renderer.display, renderer.surface, EGL_HEIGHT, &height);

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

                        Pointer p;
                        p.pos = {x, height - y - 1};
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

                            vec2 new_pos = vec2(x, height - y - 1);

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

                        input_system.pointers.erase(id);
                    }
                }
            }
            android_app_clear_motion_events(input_buffer);
        }




        if(!app->userData || !renderer.render) continue;

        if(!init) {
            init = true;

            // platform

            Transform2D t;
            t.position = vec2(0.0f);
            t.orientation = identity<mat2>();
            std::vector<vec2> square = {
                    vec2(-1, -1),
                    vec2(1, -1),
                    vec2(1, 1),
                    vec2(-1, 1)
            };

            Collider c;

            Collision_shape cs;
            cs.radius = vec2(0.0f);
            cs.vertices = square;
            for(vec2& v : cs.vertices) v *= vec2(0.5f);

            std::vector<ivec4> chunks = {
                    ivec4(-16, -1, 16, 2),
                    ivec4(-16, 2, -14, 30),
                    ivec4(14, 2, 16, 30),
                    ivec4(-16, 30, 16, 33),
                    //ivec4(-1, 8, 1, 30),
            };

            for(ivec4 chunk : chunks) {
                /*
                Collision_shape new_cs = cs;
                for(vec2& v : new_cs.vertices) v = (v + 0.5f) * vec2(chunk.z - chunk.x, chunk.w - chunk.y);
                new_cs.position = vec2(chunk.x, chunk.y);
                c.shapes.push_back(new_cs);
                */

                for(int x = chunk.x; x < chunk.z; ++x) {
                    for(int y = chunk.y; y < chunk.w; ++y) {
                        cs.position = vec2(x + 0.5f, y + 0.5f);
                        c.shapes.push_back(cs);
                    }
                }
            }

            //

            Soft_body b;

            float num = 16;

            std::vector<vec2> vs;

            for(int i = 0; i < num; ++i) {
                float angle = float(i) / num * 2 * M_PI;

                vec2 v = vec2(cos(angle), sin(angle)) * (0.75f - b.inflate);

                vs.push_back(v);
            }
            b.create(vs, vec2(0.0f, 5.0f), 40.0f);

            uint32_t entity = ecs.insert_entity();
            ecs.insert_component(entity, b);

            input_system.player_entity = entity;

            //
            {
                Transform2D transform;
                Collider collider;
                Sprite sprite;
                sprite.color_tex = core.textures["robot_big"];
                sprite.size = vec3(3.0);
                sprite.range = vec4(0.0f, 0.0f, 1.0f, 1.0f);
                sprite.offset = vec2(-1.5f, -1.0f);

                Collision_shape shape2;
                shape2.vertices = {vec2(0.0f)};
                shape2.radius = vec2(1.0f);
                shape2.mass = 40.0f;
                collider.shapes.push_back(shape2);
                collider.allow_rotation = false;
                collider.mass = 40.0f;

                transform.position = vec2(5.0f, 5.0f);
                transform.orientation = identity<mat2>();

                entity = ecs.insert_entity();
                ecs.insert_component(entity, transform);
                ecs.insert_component(entity, collider);
                ecs.insert_component(entity, sprite);
            }

            {
                Transform2D transform;
                Collider collider;
                Sprite sprite;
                sprite.color_tex = core.textures["robot_small"];
                sprite.size = vec3(1.0);
                sprite.range = vec4(0.0f, 0.0f, 1.0f, 1.0f);
                sprite.offset = vec2(-0.5f, -0.5f);

                Collision_shape shape2;
                shape2.vertices = {vec2(0.0f)};
                shape2.radius = vec2(0.5f);
                shape2.mass = 40.0f;
                collider.shapes.push_back(shape2);
                collider.allow_rotation = false;
                collider.mass = 40.0f;

                transform.position = vec2(8.0f, 5.0f);
                transform.orientation = identity<mat2>();

                entity = ecs.insert_entity();
                ecs.insert_component(entity, transform);
                ecs.insert_component(entity, collider);
                ecs.insert_component(entity, sprite);
            }

            //

            c.is_static = true;

            c.create_BVH();

            Mesh m;
            m.color = hsv_color(0.0f, 0.0f, 0.5f);
            create_mesh(m, c);


            entity = ecs.insert_entity();

            ecs.insert_component(entity, m);
            ecs.insert_component(entity, t);
            ecs.insert_component(entity, c);

            auto insert_shape = [&](vec2 pos, vec2 size, std::vector<vec2> origins, vec3 color, mat2 ori) {
                uint32_t entity = ecs.insert_entity();

                Transform2D t;
                t.position = pos;
                t.orientation = ori;


                Collider c;

                Collision_shape cs;
                cs.vertices = {
                        vec2(-1, -1),
                        vec2(1, -1),
                        vec2(1, 1),
                        vec2(-1, 1)
                };
                for(vec2& v : cs.vertices) v *= size * 0.5f;
                cs.radius = vec2(0.0f);
                cs.mass = size.x * size.y * 25.0f;

                for(vec2 v : origins) {
                    cs.position = v * size;
                    c.shapes.push_back(cs);
                }

                vec2 shift = Physics_system::calculate_inertia(c);
                t.position += shift;

                Mesh m;
                m.color = color;
                create_mesh(m, c);

                ecs.insert_component(entity, m);
                ecs.insert_component(entity, t);
                ecs.insert_component(entity, c);

                return entity;
            };

            auto insert_square = [&](vec2 pos, vec3 color, vec2 size, mat2 ori, bool allow_rotation = true) {
                uint32_t entity = ecs.insert_entity();

                Transform2D t;
                t.position = pos;
                t.orientation = ori;


                Collider c;

                Collision_shape cs;
                cs.vertices = {
                        vec2(-1, -1),
                        vec2(1, -1),
                        vec2(1, 1),
                        vec2(-1, 1)
                };
                for(vec2& v : cs.vertices) v *= size * 0.5f;
                cs.radius = vec2(0.0f);
                cs.mass = size.x * size.y * 25.0f;
                c.shapes.push_back(cs);

                c.allow_rotation = allow_rotation;

                vec2 shift = Physics_system::calculate_inertia(c);
                t.position += shift;

                Mesh m;
                m.color = color;
                create_mesh(m, c);

                ecs.insert_component(entity, m);
                ecs.insert_component(entity, t);
                ecs.insert_component(entity, c);

                return entity;
            };

            auto insert_circle = [&](vec2 pos, vec3 color, float size, mat2 ori, bool allow_rotation = true) {
                uint32_t entity = ecs.insert_entity();

                Transform2D t;
                t.position = pos;
                t.orientation = ori;


                Collider c;

                Collision_shape cs;
                cs.vertices = {
                        vec2(0.0f),
                };

                cs.radius = vec2(size);
                cs.mass = size * size * 25.0f;
                c.shapes.push_back(cs);

                c.allow_rotation = allow_rotation;

                vec2 shift = Physics_system::calculate_inertia(c);
                //t.position += shift;

                Mesh m;
                m.color = color;
                create_mesh(m, c);

                ecs.insert_component(entity, m);
                ecs.insert_component(entity, t);
                ecs.insert_component(entity, c);

                return entity;
            };

            //input_system.player_entity = insert_circle(vec2(-3.0f, 3.0f), hsv_color(287.0f / 360.0f, 0.7f, 0.9f), 0.5f, identity<mat2>(), true);

            vec2 origin = vec2(0.0f, 16.0f);
            float sep = 0.25f;
            uint32_t square_size = 6;
            mat2 ori = glm::rotate(float(M_PI * 0.33333f), vec3(0.0f, 0.0f, 1.0f));

            //
            vec2 size = vec2(1.0f);
            std::vector<vec2> origins = {
                vec2(-2, 0),
                vec2(-1, 0),
                vec2(0, 0),
                vec2(1, 0),
                vec2(2, 0),

                //

                vec2(0, -2),
                vec2(0, -1),
                vec2(0, 1),
                vec2(0, 2),
            };

            std::vector<vec2> new_origins;
            for(vec2 v : origins) {
                vec2 o = v * 4.0f - 2.0f;

                for(int x = 0; x <= 3; ++x) {
                    for (int y = 0; y <= 3; ++y) {
                        new_origins.push_back(o + vec2(x, y));
                    }
                }
            }

            origins = new_origins;

            uint32_t shape = insert_shape(origin, size, origins, hsv_color(0.5f / 6.0f, 0.7f, 0.9f), ori);

            Constraint constraint;
            pos_constraint pc;
            pc.a = vec2(0.0f, 0.0f);
            pc.b = origin;
            pc.vs = {vec2(1.0f, 0.0f), vec2(0.0f, 1.0f)};
            constraint.a = shape;
            constraint.b = NULL_ENTITY;
            constraint.pos.push_back(pc);

            physics_system.constraints.push_back(constraint);

            /*

            for(int y = 0; y < square_size; ++y) {
                for (int x = 0; x < square_size; ++x) {
                    vec2 s = vec2(float(x) - square_size * 0.5f, float(y) - square_size * 0.5f) * (1.0f + sep);

                    s = ori * s;

                    insert_square(origin + s, hsv_color(core.random() , 0.7f, 0.9f), vec2(1.0f), ori);
                }
            }

            vec3 color = hsv_color(0.5f / 6.0f, 0.7f, 0.9f);

            */

            //chain

            /*
            {
                vec2 center = vec2(4.0f, 16.0f);

                std::set<uint32_t> non_colliding;
                Physics_system& ps = ecs.get_system<Physics_system>();

                uint32_t num_links = 8;

                float scale = 1.5f;

                float sep = 0.025f * scale;
                vec2 size = vec2(0.33f, 1.0f) * scale;

                Transform2D t;
                t.position = center;
                vec2 up = normalize(vec2(1.0f, 1.0f));
                t.orientation = {up, vec2(-up.y, up.x)};

                Collider c;

                Collision_shape cs;
                cs.vertices = {vec2(0, -(size.y - size.x) * 0.5f), vec2(0.0f, (size.y - size.x) * 0.5f)};
                cs.radius = vec2(size.x * 0.5f);
                cs.mass = 0x6 * scale * scale;
                c.shapes.push_back(cs);

                vec2 shift = Physics_system::calculate_inertia(c);
                t.position += t.orientation * shift;
                t.position += t.orientation * vec2(0, size.y * 0.5f);
                //c.allow_rotation = false;

                Mesh m;
                m.color = hsv_color(0.5f / 6.0f, 0.7f, 0.9f);
                create_mesh(m, c);

                uint32_t prev_entity = NULL_ENTITY;
                uint32_t first_entity;

                for(int i = 0; i < num_links; ++i) {
                    uint32_t capsule = ecs.insert_entity();
                    non_colliding.emplace(capsule);

                    ecs.insert_component(capsule, m);
                    ecs.insert_component(capsule, t);
                    ecs.insert_component(capsule, c);


                    if(prev_entity != NULL_ENTITY) {
                        Constraint constraint;
                        constraint.a = prev_entity;
                        constraint.b = capsule;

                        pos_constraint pc;
                        pc.a = vec2(0, (size.y + sep) * 0.5f);
                        pc.b = vec2(0, -(size.y + sep) * 0.5f);
                        pc.vs = {vec2(1, 0), vec2(0, 1)};

                        constraint.pos.push_back(pc);

                        ps.constraints.push_back(constraint);
                    } else first_entity = capsule;

                    t.position += t.orientation * vec2(0, (size.y + sep));

                    prev_entity = capsule;
                }

                for(uint32_t link : non_colliding) {
                    Collider& c = ecs.get_component<Collider>(link);
                    c.non_colliding = non_colliding;
                }

                float asteroid_radius = 0.5f * scale;

                Collider c2;

                Collision_shape cs2;
                cs2.vertices = {vec2(0, 0)};
                cs2.radius = vec2(asteroid_radius);
                cs2.mass = 0x30 * scale * scale;
                c2.shapes.push_back(cs2);

                shift = Physics_system::calculate_inertia(c2);
                t.position += t.orientation * shift;
                t.position += t.orientation * vec2(0, 1.0f);

                Mesh m2;
                m2.color = hsv_color(0.5f / 6.0f, 0.7f, 0.9f);
                create_mesh(m2, c2);

                t.position = center + t.orientation * vec2(0, (size.y + sep) * num_links + asteroid_radius);

                uint32_t asteroid = ecs.insert_entity();
                ecs.insert_component(asteroid, m2);
                ecs.insert_component(asteroid, t);
                ecs.insert_component(asteroid, c2);

                {
                    Constraint constraint;
                    constraint.a = prev_entity;
                    constraint.b = asteroid;

                    pos_constraint pc;
                    pc.a = vec2(0, (size.y + sep) * 0.5f);
                    pc.b = vec2(0, -(asteroid_radius + sep * 0.5f));
                    pc.vs = {vec2(1, 0), vec2(0, 1)};

                    constraint.pos.push_back(pc);

                    ps.constraints.push_back(constraint);
                }

                //

                t.position = center + t.orientation * -vec2(0, asteroid_radius);

                asteroid = ecs.insert_entity();
                ecs.insert_component(asteroid, m2);
                ecs.insert_component(asteroid, t);
                ecs.insert_component(asteroid, c2);

                {
                    Constraint constraint;
                    constraint.a = asteroid;
                    constraint.b = first_entity;

                    pos_constraint pc;
                    pc.a = vec2(0, asteroid_radius + sep * 0.5f);
                    pc.b = vec2(0, -(size.y + sep) * 0.5f);
                    pc.vs = {vec2(1, 0), vec2(0, 1)};

                    constraint.pos.push_back(pc);

                    ps.constraints.push_back(constraint);
                }
            }
            */
        }

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
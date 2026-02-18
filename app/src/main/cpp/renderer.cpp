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
#include "physics.h"

#include <GLES/egl.h>

using namespace glm;

struct Object_vertex {
    vec3 v;
};

void create_mesh(Mesh& m, std::vector<vec2> v, vec2 radius, bool create_interior) {
    m.v_tris = std::shared_ptr<Vertices>(new Vertices);
    m.v_tris->init();

    m.v_lines = std::shared_ptr<Vertices>(new Vertices);
    m.v_lines->init();

    float sphere_segments = max(64, int32_t(8 * max(radius.x, radius.y)));

    std::vector<vec2> vv;
    if(radius.x == 0.0f && radius.y == 0.0f) {
        for(int i = 0; i < v.size(); ++i) {
            vv.push_back(v[i]);
        }
    } else {
        vec2 sum = vec2(0.0f);
        for(int i = 0; i < v.size(); ++i) {
            sum += v[i];
        }
        sum /= v.size();

        std::unordered_set<uint32_t> cc;
        vec2 first_normal = vec2(0.0f);
        vec2 prev_normal = vec2(0.0f);
        for(int i = 0; i < v.size(); ++i) {
            uint16_t a = i;
            uint16_t b = (i + 1) % v.size();
            if(a == b) {
                vec2 v0 = v[a];

                float angle_a = 0.0f;
                float angle_b = 2 * M_PI;

                float angle_per_segment = 2 * M_PI / sphere_segments;

                for(float j = angle_a + angle_per_segment; j < angle_b; j += angle_per_segment) {
                    vec2 vc = {cos(j), sin(j)};
                    vc *= radius;
                    vc = v0 + vc;

                    vv.push_back(vc);
                }
            } else {
                vec2 v0 = v[a];
                vec2 v1 = v[b];

                if(a > b) {
                    uint16_t temp = a;
                    a = b;
                    b = temp;
                }

                vec2 direction = normalize(v[b] - v[a]);
                vec2 normal = vec2(direction.y, -direction.x);
                if(dot(normal, sum - v[a]) > 0.0f) {
                    normal = -normal;
                }

                uint32_t c = (uint32_t)a | ((uint32_t)b << 16);
                if(cc.find(c) != cc.end()) {
                    normal = -normal;
                } else {
                    cc.insert(c);
                }

                // create previous sphere
                if(prev_normal.x != 0.0f || prev_normal.y != 0.0f) {
                    vec2 va = prev_normal * radius;
                    vec2 vb = normal * radius;
                    float angle_a = atan2(va.y, va.x);
                    float angle_b = atan2(vb.y, vb.x);

                    if(angle_b < angle_a) angle_a -= 2 * M_PI;

                    float angle_per_segment = 2 * M_PI / sphere_segments;

                    for(float j = angle_a + angle_per_segment; j < angle_b; j += angle_per_segment) {
                        vec2 vc = {cos(j), sin(j)};
                        vc *= radius;
                        vc = v0 + vc;

                        vv.push_back(vc);
                    }
                }

                vv.push_back(v0 + normal * radius);
                vv.push_back(v1 + normal * radius);

                // create first sphere
                if(i == v.size() - 1) {
                    vec2 va = normal * radius;
                    vec2 vb = first_normal * radius;
                    float angle_a = atan2(va.y, va.x);
                    float angle_b = atan2(vb.y, vb.x);

                    if(angle_b < angle_a) angle_a -= 2 * M_PI;

                    float angle_per_segment = 2 * M_PI / sphere_segments;

                    for(float j = angle_a + angle_per_segment; j < angle_b; j += angle_per_segment) {
                        vec2 vc = {cos(j), sin(j)};
                        vc *= radius;
                        vc = v1 + vc;

                        vv.push_back(vc);
                    }
                }


                prev_normal = normal;
                if(first_normal.x == 0.0f && first_normal.y == 0.0f) first_normal = normal;
            }
        }
    }

    float min_dist = FLT_MAX;
    std::vector<Object_vertex> vvv;
    for(int i = 0 ; i < vv.size(); ++i) {
        vec2 v0 = vv[i];
        vec2 v1 = vv[(i + 1) % vv.size()];

        vec2 origin_v = -v0;
        float len = length(v1 - v0);
        vec2 dir = (v1 - v0) / len;
        float f = dot(dir, origin_v);
        f = clamp(f, 0.0f, len);
        vec2 closest_point = f * dir + v0;

        min_dist = min(length(closest_point), min_dist);



        Object_vertex ov;
        ov.v = vec3(v0, 0.5);
        vvv.push_back(ov);

        ov.v = vec3(v1, 0.5);
        vvv.push_back(ov);
    }

    /*
    Object_vertex ov;
    ov.v = vec3(0.0f, 0.0f, 0.5);
    vvv.push_back(ov);
    ov.v = vec3(vec2(0.0f, 0.75f) * min_dist, 0.5);
    vvv.push_back(ov);
    */

    m.v_lines->vertex_buffer_data(vvv.data(), vvv.size(), sizeof(Object_vertex), GL_STATIC_DRAW);
    m.v_lines->add_vertex_attribute(0, 3, GL_FLOAT, false, sizeof(float) * 3, 0);

    vvv.clear();

    if(create_interior) {
        for(int i = 0; i < vv.size(); ++i) {
            vec3 v0 = vec3(vv[i], 0.5f);
            vec3 v1 = vec3(vv[(i + 1) % vv.size()], 0.5f);
            vec3 v2 = vec3(0.0f, 0.0f, 0.5f);

            vec3 vc = cross(v0 - v2, v1 - v2);
            if(vc.z < 0.0f) {
                vec3 temp = v1;
                v1 = v2;
                v2 = temp;
            }

            Object_vertex ov;
            ov.v = v0;
            vvv.push_back(ov);

            ov.v = v1;
            vvv.push_back(ov);

            ov.v = v2;
            vvv.push_back(ov);
        }
    }

    m.v_tris->vertex_buffer_data(vvv.data(), vvv.size(), sizeof(Object_vertex), GL_STATIC_DRAW);
    m.v_tris->add_vertex_attribute(0, 3, GL_FLOAT, false, sizeof(float) * 3, 0);
}

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
    c.radius = vec2(0.0f);
    c.vertices = square;
    for(vec2& v : c.vertices) v *= vec2(256, 16);

    c.is_static = true;

    Mesh m;
    create_mesh(m, {c.vertices}, c.radius);

    uint32_t entity = ecs.insert_entity();

    ecs.insert_component(entity, m);
    ecs.insert_component(entity, t);
    ecs.insert_component(entity, c);

    auto insert_square = [&](vec2 pos, vec2 size, mat2 ori) {
        uint32_t entity = ecs.insert_entity();

        Transform2D t;
        t.position = pos;
        t.orientation = ori;


        Collider c;
        c.vertices = {
                vec2(-1, -1),
                vec2(1, -1),
                vec2(1, 1),
                vec2(-1, 1)
        };
        for(vec2& v : c.vertices) v *= size * 0.5f;
        c.radius = vec2(0.0f);
        c.mass = size.x * size.y * 25.0f;
        //c.allow_rotation = false;

        vec2 shift = Physics_system::calculate_inertia(c);
        t.position += shift;

        Mesh m;
        m.color = vec3(0.35f);//get_color(abs(core.random())) * 0.7f + 0.3f;
        create_mesh(m, c.vertices, c.radius);

        ecs.insert_component(entity, m);
        ecs.insert_component(entity, t);
        ecs.insert_component(entity, c);
    };

    vec2 origin = vec2(0.0f, 32.0f);
    float sep = 0.25f;
    uint32_t square_size = 5;
    mat2 ori = glm::rotate(float(M_PI * 0.33333f), vec3(0.0f, 0.0f, 1.0f));

    for(int y = 0; y < square_size; ++y) {
        for (int x = 0; x < square_size; ++x) {
            vec2 s = vec2(float(x) - square_size * 0.5f, float(y) - square_size * 0.5f) * (1.0f + sep);

            s = ori * s;

            insert_square(origin + s, vec2(1.0f), ori);
        }
    }

    //chain
    {
        vec2 center = vec2(0.0f, 48.0f);

        std::set<uint32_t> non_colliding;
        Physics_system& ps = ecs.get_system<Physics_system>();

        uint32_t num_links = 16;

        float scale = 1.5f;

        float sep = 0.025f * scale;
        vec2 size = vec2(0.33f, 1.0f) * scale;

        Transform2D t;
        t.position = center;
        vec2 up = normalize(vec2(1.0f, 1.0f));
        t.orientation = {up, vec2(-up.y, up.x)};

        Collider c;
        c.vertices = {vec2(0, -(size.y - size.x) * 0.5f), vec2(0.0f, (size.y - size.x) * 0.5f)};
        c.radius = vec2(size.x * 0.5f);
        c.mass = 0x6 * scale * scale;
        vec2 shift = Physics_system::calculate_inertia(c);
        t.position += t.orientation * shift;
        t.position += t.orientation * vec2(0, size.y * 0.5f);
        //c.allow_rotation = false;

        Mesh m;
        m.color = vec3(0.9f, 0.9f, 0.9f);
        create_mesh(m, c.vertices, c.radius);

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
        c2.vertices = {vec2(0, 0)};
        c2.radius = vec2(asteroid_radius);
        c2.mass = 0x30 * scale * scale;
        shift = Physics_system::calculate_inertia(c2);
        t.position += t.orientation * shift;
        t.position += t.orientation * vec2(0, 1.0f);

        Mesh m2;
        m2.color = vec3(0.9f, 0.9f, 0.9f);
        create_mesh(m2, c2.vertices, c2.radius);

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
}

Renderer::Renderer() {
    Signature s = ecs.update_signature<Transform2D>();
    ecs.update_signature<Mesh>(s);
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

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(!vertices) {
        vertices = std::shared_ptr<Vertices>(new Vertices);
        vertices->init();
    }

    glDisable(GL_DEPTH_TEST);

    render_background();

    //render_text();

    for(uint32_t entity : collectors[0].entities) render_mesh(entity);

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

void Renderer::render_mesh(uint32_t entity) {
    Transform2D& ct = ecs.get_component<Transform2D>(camera);
    Camera2D& cc = ecs.get_component<Camera2D>(camera);

    mat4 inv_rot = mat4(transpose(ct.orientation));

    mat4 view = inv_rot * scale(vec3(cc.scale, cc.scale, 1.0f)) * translate(vec3(-ct.position, 0.0f));

    int width, height;
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);

    float aspect_ratio = float(height) / width;
    mat4 proj = scale(vec3(1.0f, 1.0f / aspect_ratio, 1.0f));

    Transform2D& mesh_t = ecs.get_component<Transform2D>(entity);
    Mesh& mesh_m = ecs.get_component<Mesh>(entity);

    mat4 model = translate(vec3(mesh_t.position, 0.0f)) * mat4(mesh_t.orientation);

    core.shaders["color"]->use();

    mesh_m.v_tris->bind();

    glUniformMatrix4fv(0, 1, false, &proj[0][0]);
    glUniformMatrix4fv(1, 1, false, &view[0][0]);
    glUniformMatrix4fv(2, 1, false, &model[0][0]);
    glUniform4f(3, mesh_m.color.x, mesh_m.color.y, mesh_m.color.z, 0.2f);

    mesh_m.v_tris->draw_vertices(GL_TRIANGLES);

    glLineWidth(2);
    core.shaders["color"]->use();

    mesh_m.v_lines->bind();

    glUniformMatrix4fv(0, 1, false, &proj[0][0]);
    glUniformMatrix4fv(1, 1, false, &view[0][0]);
    glUniformMatrix4fv(2, 1, false, &model[0][0]);
    glUniform4f(3, mesh_m.color.x, mesh_m.color.y, mesh_m.color.z, 1.0f);

    mesh_m.v_lines->draw_vertices(GL_LINES);
}
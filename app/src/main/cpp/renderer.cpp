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
#include "levels.h"

#include <GLES/egl.h>

using namespace glm;

struct Object_vertex {
    vec3 position;
    vec3 normal;
};

/*
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
*/

std::vector<vec2> get_mesh(std::vector<vec2> v, vec2 radius) {
    float sphere_segments = 32;

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

                for(float j = angle_a; j <= angle_b; j += angle_per_segment) {
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

                    for(float j = angle_a; j <= angle_b; j += angle_per_segment) {
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

                    for(float j = angle_a; j <= angle_b; j += angle_per_segment) {
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

    return vv;
}

void create_mesh(Mesh& m, std::vector<vec2> v, vec2 radius) {
    if(m.v_lines) {
        m.v_lines->vertex_buffer = 0;
        m.v_lines->index_buffer = 0;
        m.v_lines->vertex_array = 0;
    }
    if(m.v_tris) {
        m.v_tris->vertex_buffer = 0;
        m.v_tris->index_buffer = 0;
        m.v_tris->vertex_array = 0;
    }

    m.v_lines = std::shared_ptr<Vertices>(new Vertices);
    m.v_lines->init();
    m.v_tris = std::shared_ptr<Vertices>(new Vertices);
    m.v_tris->init();

    //

    auto vvv = get_mesh(v, radius);

    vec2 center = vec2(0.0f);
    for(vec2 v : vvv) {
        center += v;
    }
    center /= float(vvv.size());

    //

    std::vector<Object_vertex> lines;
    std::vector<Object_vertex> tris;

    Object_vertex c = {vec3(center, 0.5f), vec3(0.0f, 0.0f, 1.0f)};

    for(int i = 0; i < vvv.size(); ++i) {
        Object_vertex a = {vec3(vvv[i], 0.5f), vec3(0.0f, 0.0f, 1.0f)};
        Object_vertex b = {vec3(vvv[(i + 1) % vvv.size()], 0.5f), vec3(0.0f, 0.0f, 1.0f)};

        lines.push_back(a);
        lines.push_back(b);

        tris.push_back(a);
        tris.push_back(b);
        tris.push_back(c);
    }

    m.v_lines->vertex_buffer_data(lines.data(), lines.size(), sizeof(Object_vertex), GL_STATIC_DRAW);
    m.v_lines->add_vertex_attribute(0, 3, GL_FLOAT, false, sizeof(float) * 6, 0);
    m.v_lines->add_vertex_attribute(1, 3, GL_FLOAT, false, sizeof(float) * 6, sizeof(float) * 3);

    m.v_tris->vertex_buffer_data(tris.data(), tris.size(), sizeof(Object_vertex), GL_STATIC_DRAW);
    m.v_tris->add_vertex_attribute(0, 3, GL_FLOAT, false, sizeof(float) * 6, 0);
    m.v_tris->add_vertex_attribute(1, 3, GL_FLOAT, false, sizeof(float) * 6, sizeof(float) * 3);
}

void create_mesh(Mesh& m, Collider& c) {
    if(m.v_lines) {
        m.v_lines->vertex_buffer = 0;
        m.v_lines->index_buffer = 0;
        m.v_lines->vertex_array = 0;
    }
    if(m.v_tris) {
        m.v_tris->vertex_buffer = 0;
        m.v_tris->index_buffer = 0;
        m.v_tris->vertex_array = 0;
    }

    m.v_lines = std::shared_ptr<Vertices>(new Vertices);
    m.v_lines->init();
    m.v_tris = std::shared_ptr<Vertices>(new Vertices);
    m.v_tris->init();

    //

    std::vector<Object_vertex> lines;
    std::vector<Object_vertex> tris;

    for(Collision_shape& cs : c.shapes) {
        std::vector<vec2> vs = get_mesh(cs.vertices, cs.radius);

        vec2 center = vec2(0.0f);
        for(vec2& v : vs) {
            v = cs.orientation * v + cs.position;
            center += v;
        }
        center /= float(vs.size());

        for(int i = 0; i < vs.size(); ++i) {
            vec2 v0 = vs[i];
            vec2 v1 = vs[(i + 1) % vs.size()];

            lines.push_back(Object_vertex{vec3(v0, 0.5), vec3(0.0f, 0.0f, 1.0f)});
            lines.push_back(Object_vertex{vec3(v1, 0.5), vec3(0.0f, 0.0f, 1.0f)});

            tris.push_back(Object_vertex{vec3(v0, 0.5), vec3(0.0f, 0.0f, 1.0f)});
            tris.push_back(Object_vertex{vec3(v1, 0.5), vec3(0.0f, 0.0f, 1.0f)});
            tris.push_back(Object_vertex{vec3(center, 0.5), vec3(0.0f, 0.0f, 1.0f)});
        }
    }

    //

    m.v_lines->vertex_buffer_data(lines.data(), lines.size(), sizeof(Object_vertex), GL_STATIC_DRAW);
    m.v_lines->add_vertex_attribute(0, 3, GL_FLOAT, false, sizeof(float) * 6, 0);
    m.v_lines->add_vertex_attribute(1, 3, GL_FLOAT, false, sizeof(float) * 6, sizeof(float) * 3);

    m.v_tris->vertex_buffer_data(tris.data(), tris.size(), sizeof(Object_vertex), GL_STATIC_DRAW);
    m.v_tris->add_vertex_attribute(0, 3, GL_FLOAT, false, sizeof(float) * 6, 0);
    m.v_tris->add_vertex_attribute(1, 3, GL_FLOAT, false, sizeof(float) * 6, sizeof(float) * 3);
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

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // framebuffers

    int width, height;
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);

    int pixel_size = 1;
    ivec2 fb_size = ivec2(width, height);

    std::shared_ptr<Framebuffer> fb0(new Framebuffer(fb_size, {{{GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}, GL_COLOR_ATTACHMENT0, 0}}));
    framebuffers.push_back(fb0);

    fb_size = ivec2(64);
    std::shared_ptr<Framebuffer> fb1(new Framebuffer(fb_size, {{{GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}, GL_COLOR_ATTACHMENT0, 0}}));
    framebuffers.push_back(fb1);
}

Renderer::Renderer() {
    Signature s = ecs.update_signature<Transform2D>();
    ecs.update_signature<Mesh>(s);
    collectors.push_back(Collector{s, false});

    s = ecs.update_signature<Transform2D>();
    ecs.update_signature<Sprite>(s);
    collectors.push_back(Collector{s, false});

    s = ecs.update_signature<Soft_body>();
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
    // screen
    int width, height;
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);

    screen_size = {width, height};


    // calculate camera view

    vec2 ratio;
    if(width > height) ratio = vec2(1.0f, float(height) / width);
    else ratio = vec2(float(width) / height, 1.0f);

    // camera
    Level_system& ls = ecs.get_system<Level_system>();
    Input_system& is = ecs.get_system<Input_system>();

    Camera2D& camera_camera = ecs.get_component<Camera2D>(camera);
    Transform2D& camera_transform = ecs.get_component<Transform2D>(camera);

    Soft_body& player_soft_body = ecs.get_component<Soft_body>(is.player_entity);
    //Collider& player_collider = ecs.get_component<Collider>(is.player_entity);
    //Transform2D& player_transform = ecs.get_component<Transform2D>(is.player_entity);

    vec2 screen_size = ratio * 2.0f / camera_camera.scale;
    vec4 center_region = vec4(camera_transform.position - screen_size * 0.125f, camera_transform.position + screen_size * 0.125f);

    vec2 new_pos = camera_transform.position;

    if(player_soft_body.target_center.x < center_region.x) {
        new_pos.x += player_soft_body.target_center.x - center_region.x;
    } else if(player_soft_body.target_center.x > center_region.z) {
        new_pos.x += player_soft_body.target_center.x - center_region.z;
    }

    if(player_soft_body.target_center.y < center_region.y) {
        new_pos.y += player_soft_body.target_center.y - center_region.y;
    } else if(player_soft_body.target_center.y > center_region.w) {
        new_pos.y += player_soft_body.target_center.y - center_region.w;
    }

    vec4 new_range = vec4(new_pos - screen_size * 0.5f, new_pos + screen_size * 0.5f);

    if(new_range.x < ls.range.x) {
        new_pos.x += ls.range.x - new_range.x;
    }
    if(new_range.y < ls.range.y) {
        new_pos.y += ls.range.y - new_range.y;
    }
    if(new_range.z > ls.range.z) {
        new_pos.x += ls.range.z - new_range.z;
    }
    if(new_range.w > ls.range.w) {
        new_pos.y += ls.range.w - new_range.w;
    }

    camera_transform.position = new_pos;

    //

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);
    glEnable(GL_BLEND);

    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE);
    glClearDepthf(0.0f);
    //glDepthRangef(1.0f, 0.0f);


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);

    camera_camera.proj = scale(vec3(1.0f / ratio, 1.0f));

    if(!vertices) {
        vertices = std::shared_ptr<Vertices>(new Vertices);
        vertices->init();
    }

    glDisable(GL_DEPTH_TEST);

    render_background();

    //render_text();

    for(uint32_t entity : collectors[0].entities) render_mesh(entity);
    for(uint32_t entity : collectors[1].entities) render_sprite(entity);
    for(uint32_t entity : collectors[2].entities) render_slime(entity);
    //
    render_points();

    //framebuffers[0]->bind();
    //glViewport(0, 0, framebuffers[0]->size.x, framebuffers[0]->size.y);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render_buttons();

    //


    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glViewport(0, 0, width, height);

    //core.shaders["screen"]->use();
    //framebuffers[0]->textures[0].bind(0);
    //glDrawArrays(GL_TRIANGLES, 0, 6);

    //

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

    float aspect_ratio = float(screen_size.y) / screen_size.x;
    mat4 proj = cc.proj;

    core.shaders["background"]->use();

    vertices->bind();

    glUniformMatrix4fv(0, 1, false, &view[0][0]);
    glUniformMatrix4fv(1, 1, false, &proj[0][0]);
    glUniform2f(2, float(screen_size.x), float(screen_size.y));

    vertices->draw_vertices(GL_TRIANGLES);
}

void Renderer::render_soft_body(uint32_t entity) {
    Soft_body& soft_body = ecs.get_component<Soft_body>(entity);

    //

    std::vector<Object_vertex> lines;
    std::vector<Object_vertex> tris;

    vec2 center = vec2(0.0f);
    for(auto p : soft_body.points) {
        center += p.position;
    }
    center /= float(soft_body.points.size());

    for(int i = 0; i < soft_body.points.size(); ++i) {
        vec2 a = soft_body.points[i].position;
        vec2 b = soft_body.points[(i + 1) % soft_body.points.size()].position;

        Object_vertex v;
        v.normal = vec3(0.0f, 0.0f, 1.0f);

        v.position = vec3(a, 0.5f);
        lines.push_back(v);
        v.position = vec3(b, 0.5f);
        lines.push_back(v);

        v.position = vec3(a, 0.5f);
        tris.push_back(v);
        v.position = vec3(b, 0.5f);
        tris.push_back(v);
        v.position = vec3(center, 0.5f);
        tris.push_back(v);
    }

    //

    Transform2D& ct = ecs.get_component<Transform2D>(camera);
    Camera2D& cc = ecs.get_component<Camera2D>(camera);

    mat4 inv_rot = mat4(transpose(ct.orientation));

    mat4 view = inv_rot * scale(vec3(cc.scale, cc.scale, 1.0f)) * translate(vec3(-ct.position, 0.0f));

    float aspect_ratio = float(screen_size.y) / screen_size.x;
    mat4 proj = cc.proj;

    mat4 model = identity<mat4>();

    core.shaders["shape"]->use();

    vec3 light = vec3(0.5f, -0.5f, 1.0f);

    glLineWidth(2);

    vec3 color = vec3(1.0f, 0.25f, 0.25f);

    //

    vertices->vertex_buffer_data(tris.data(), tris.size(), sizeof(Object_vertex), GL_STATIC_DRAW);
    vertices->add_vertex_attribute(0, 3, GL_FLOAT, false, sizeof(float) * 6, 0);
    vertices->add_vertex_attribute(1, 3, GL_FLOAT, false, sizeof(float) * 6, sizeof(float) * 3);
    vertices->bind();

    glUniformMatrix4fv(0, 1, false, &proj[0][0]);
    glUniformMatrix4fv(1, 1, false, &view[0][0]);
    glUniformMatrix4fv(2, 1, false, &model[0][0]);
    glUniform4f(3, color.x, color.y, color.z,  0.35f);
    glUniform3fv(4, 1, &light[0]);

    vertices->draw_vertices(GL_TRIANGLES);

    //
    vertices->vertex_buffer_data(lines.data(), lines.size(), sizeof(Object_vertex), GL_STATIC_DRAW);
    vertices->add_vertex_attribute(0, 3, GL_FLOAT, false, sizeof(float) * 6, 0);
    vertices->add_vertex_attribute(1, 3, GL_FLOAT, false, sizeof(float) * 6, sizeof(float) * 3);
    vertices->bind();

    glUniformMatrix4fv(0, 1, false, &proj[0][0]);
    glUniformMatrix4fv(1, 1, false, &view[0][0]);
    glUniformMatrix4fv(2, 1, false, &model[0][0]);
    glUniform4f(3, color.x, color.y, color.z, 1.0f);
    glUniform3fv(4, 1, &light[0]);

    vertices->draw_vertices(GL_LINES);



}

void Renderer::render_gui() {
    GUI_system& gui = ecs.get_system<GUI_system>();
    Transform2D& ct = ecs.get_component<Transform2D>(camera);
    Camera2D& cc = ecs.get_component<Camera2D>(camera);

    float text_scale = 3.0f;

    mat4 inv = scale(vec3(1.0f, -1.0f, 1.0f));

    mat4 proj = translate(vec3(-1.0f, -1.0f, 0.0f)) * scale(vec3(2.0f / float(screen_size.x), 2.0f / float(screen_size.y), 1.0f));
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

    float aspect_ratio = float(screen_size.y) / screen_size.x;
    mat4 proj = cc.proj;

    Transform2D& mesh_t = ecs.get_component<Transform2D>(entity);
    Mesh& mesh_m = ecs.get_component<Mesh>(entity);

    mat4 model = translate(vec3(mesh_t.position, 0.0f)) * mat4(mesh_t.orientation);

    core.shaders["shape"]->use();

    vec3 light = vec3(0.5f, -0.5f, 1.0f);

    glLineWidth(2);

    //

    mesh_m.v_tris->bind();

    glUniformMatrix4fv(0, 1, false, &proj[0][0]);
    glUniformMatrix4fv(1, 1, false, &view[0][0]);
    glUniformMatrix4fv(2, 1, false, &model[0][0]);
    glUniform4f(3, mesh_m.color.x, mesh_m.color.y, mesh_m.color.z,  0.35f);
    glUniform3fv(4, 1, &light[0]);

    mesh_m.v_tris->draw_vertices(GL_TRIANGLES);

    //

    mesh_m.v_lines->bind();

    glUniformMatrix4fv(0, 1, false, &proj[0][0]);
    glUniformMatrix4fv(1, 1, false, &view[0][0]);
    glUniformMatrix4fv(2, 1, false, &model[0][0]);
    glUniform4f(3, mesh_m.color.x, mesh_m.color.y, mesh_m.color.z, 1.0f);
    glUniform3fv(4, 1, &light[0]);

    mesh_m.v_lines->draw_vertices(GL_LINES);
}

void Renderer::render_sprite(uint32_t entity) {
    Transform2D& ct = ecs.get_component<Transform2D>(camera);
    Camera2D& cc = ecs.get_component<Camera2D>(camera);

    mat4 inv_rot = mat4(transpose(ct.orientation));

    mat4 view = inv_rot * scale(vec3(cc.scale, cc.scale, 1.0f)) * translate(vec3(-ct.position, 0.0f));

    float aspect_ratio = float(screen_size.y) / screen_size.x;
    mat4 proj = cc.proj;

    Transform2D& transform = ecs.get_component<Transform2D>(entity);
    Sprite& sprite = ecs.get_component<Sprite>(entity);

    mat4 model = translate(vec3(transform.position, 0.0f)) * mat4(transform.orientation);

    core.shaders["sprite"]->use();

    /*
    layout(location = 0) uniform mat4 proj;
    layout(location = 1) uniform mat4 view;
    layout(location = 2) uniform mat4 model;
    layout(location = 3) uniform vec2 size;
    layout(location = 4) uniform vec2 offset;
    layout(location = 5) uniform vec4 range;
    layout(location = 6) uniform vec3 light;
    */

    sprite.color_tex->bind(0);
    sprite.normal_tex->bind(1);

    vec3 light = vec3(0.5f, -0.5f, 1.0f);

    glUniformMatrix4fv(0, 1, false, &proj[0][0]);
    glUniformMatrix4fv(1, 1, false, &view[0][0]);
    glUniformMatrix4fv(2, 1, false, &model[0][0]);
    glUniform2fv(3, 1, &sprite.size.x); // size
    glUniform2fv(4, 1, &sprite.offset.x); // offset
    glUniform4fv(5, 1, &sprite.range.x); // range
    glUniform3fv(6, 1, &light.x);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::render_points() {
    Transform2D& ct = ecs.get_component<Transform2D>(camera);
    Camera2D& cc = ecs.get_component<Camera2D>(camera);

    mat4 inv_rot = mat4(transpose(ct.orientation));

    mat4 view = inv_rot * scale(vec3(cc.scale, cc.scale, 1.0f)) * translate(vec3(-ct.position, 0.0f));

    float aspect_ratio = float(screen_size.y) / screen_size.x;
    mat4 proj = cc.proj;

    uint32_t i = 0;

    for(vec2 p : points) {

        mat4 model = translate(vec3(p, 0.0f));

        core.shaders["sprite"]->use();

        /*
        layout(location = 0) uniform mat4 proj;
        layout(location = 1) uniform mat4 view;
        layout(location = 2) uniform mat4 model;
        layout(location = 3) uniform vec2 size;
        layout(location = 4) uniform vec2 offset;
        layout(location = 5) uniform vec4 range;
        layout(location = 6) uniform vec3 light;
        */

        core.textures["gui"]->bind(0);

        vec3 light = vec3(0.5f, -0.5f, 1.0f);

        glUniformMatrix4fv(0, 1, false, &proj[0][0]);
        glUniformMatrix4fv(1, 1, false, &view[0][0]);
        glUniformMatrix4fv(2, 1, false, &model[0][0]);
        glUniform2f(3, 0.125f, 0.125f); // size
        glUniform2f(4, -0.0625f, -0.0625f); // offset
        glUniform4f(5, 0.0f, 2.0f / 64.0f, 5.0f / 64.0f, 5.0f / 64.0f); // range
        glUniform3fv(6, 1, &light.x);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        //

        std::vector<vec3> vs = {
                vec3(0.0f, 0.0f, 0.5f),
                vec3(normals[i] * 0.375f, 0.5f)
        };

        vertices->vertex_buffer_data(vs.data(), vs.size(), sizeof(vec3), GL_STREAM_DRAW);
        vertices->add_vertex_attribute(0, 3, GL_FLOAT, false, sizeof(vec3), 0);

        core.shaders["color"]->use();
        vertices->bind();

        /*
        layout(location = 0) uniform mat4 proj;
        layout(location = 1) uniform mat4 view;
        layout(location = 2) uniform mat4 model;
        layout(location = 3) uniform vec4 color;
         */

        glUniformMatrix4fv(0, 1, false, &proj[0][0]);
        glUniformMatrix4fv(1, 1, false, &view[0][0]);
        glUniformMatrix4fv(2, 1, false, &model[0][0]);
        glUniform4f(3, cs[i].x, cs[i].y, cs[i].z, 1.0f);

        glDrawArrays(GL_LINES, 0, 2);

        ++i;
    }
}

std::vector<vec2> get_circle(int num_points) {
    std::vector<vec2> ret;

    for(int i = 0; i < num_points; ++i) {
        float angle = 2.0f * M_PI * (float(i) / num_points);

        vec2 v = {cos(angle), sin(angle)};

        ret.push_back(v);
    }

    return ret;
}

void Renderer::render_buttons() {
    // static
    static std::vector<vec2> vs = get_circle(32);


    Input_system& input_system = ecs.get_system<Input_system>();

    // setup

    int min_dimension = min(screen_size.x, screen_size.y);

    mat4 proj = translate(vec3(-1.0f, -1.0f, 0.0f)) * scale(vec3(2.0f / float(screen_size.x), 2.0f / float(screen_size.y), 1.0f));
    mat4 view = identity<mat4>();
    mat4 model = identity<mat4>();

    // rendering

    core.shaders["color"]->use();

    glUniformMatrix4fv(0, 1, false, &proj[0][0]);
    glUniformMatrix4fv(1, 1, false, &view[0][0]);
    glUniformMatrix4fv(2, 1, false, &model[0][0]);
    glUniform4f(3, 1.0f, 1.0f, 1.0f, 1.0f);

    //

    vec2 center = vec2(min_dimension / 8.0f * 1.5f);

    // joystick ring

    std::vector<vec3> buffer;

    for(int i = 0; i < vs.size(); ++i) {
        vec2 a = vs[i] * (float(min_dimension) / 8.0f);
        vec2 b = vs[(i + 1) % vs.size()] * (float(min_dimension) / 8.0f);
        vec2 c = vs[i] * (float(min_dimension) / 8.0f - 8.0f);
        vec2 d = vs[(i + 1) % vs.size()] * (float(min_dimension) / 8.0f - 8.0f);

        buffer.push_back(vec3(a + center, 0.5f));
        buffer.push_back(vec3(b + center, 0.5f));
        buffer.push_back(vec3(d + center, 0.5f));

        buffer.push_back(vec3(a + center, 0.5f));
        buffer.push_back(vec3(d + center, 0.5f));
        buffer.push_back(vec3(c + center, 0.5f));
    }

    vertices->vertex_buffer_data(buffer.data(), buffer.size(), sizeof(vec3), GL_STREAM_DRAW);
    vertices->add_vertex_attribute(0, 3, GL_FLOAT, false, sizeof(vec3), 0);

    vertices->draw_vertices(GL_TRIANGLES);

    // joystick circle

    buffer.clear();

    vec2 pos = center + input_system.joystick * (float(min_dimension) / 8.0f);

    for(int i = 0; i < vs.size(); ++i) {
        vec2 a = vs[i] * (float(min_dimension) / 8.0f * 0.5f);
        vec2 b = vs[(i + 1) % vs.size()] * (float(min_dimension) / 8.0f * 0.5f);
        vec2 c = vec3(0.0f);

        buffer.push_back(vec3(a + pos, 0.5f));
        buffer.push_back(vec3(b + pos, 0.5f));
        buffer.push_back(vec3(c + pos, 0.5f));
    }

    vertices->vertex_buffer_data(buffer.data(), buffer.size(), sizeof(vec3), GL_STREAM_DRAW);
    vertices->add_vertex_attribute(0, 3, GL_FLOAT, false, sizeof(vec3), 0);

    vertices->draw_vertices(GL_TRIANGLES);

    // jump circle

    vec3 color = mix(vec3(1.0f, 1.0f, 1.0f), hsv_color(3.8f / 6.0f, 0.8f, 1.0f), clamp(input_system.jump, 0.0f, 1.0f));
    //glUniform4f(3, color.x, color.y, color.z, 1.0f);

    buffer.clear();

    pos = vec2(screen_size.x - min_dimension / 8.0f * 1.5f, min_dimension / 8.0f * 1.5f);
    float radius = mix(0.625f, 0.75f, clamp(input_system.jump, 0.0f, 1.0f));

    for(int i = 0; i < vs.size(); ++i) {
        vec2 a = vs[i] * (float(min_dimension) / 8.0f * radius);
        vec2 b = vs[(i + 1) % vs.size()] * (float(min_dimension) / 8.0f * radius);
        vec2 c = vec3(0.0f);

        buffer.push_back(vec3(a + pos, 0.5f));
        buffer.push_back(vec3(b + pos, 0.5f));
        buffer.push_back(vec3(c + pos, 0.5f));
    }

    vertices->vertex_buffer_data(buffer.data(), buffer.size(), sizeof(vec3), GL_STREAM_DRAW);
    vertices->add_vertex_attribute(0, 3, GL_FLOAT, false, sizeof(vec3), 0);

    vertices->draw_vertices(GL_TRIANGLES);
}


struct Tex_vertex {
    vec3 position;
    vec2 tex_coord;
};

void Renderer::render_slime(uint32_t entity) {
    static float look = 0.0f;
    static float return_look = 0.0f;

    /*
    layout(location = 0) in vec3 position;
    layout(location = 0) in vec2 tex_coord;

    layout(location = 0) uniform mat4 proj;
    layout(location = 1) uniform mat4 view;
    layout(location = 2) uniform mat4 model;
    layout(location = 3) uniform vec4 color;
     */
    Input_system& is = ecs.get_system<Input_system>();

    Soft_body& soft_body = ecs.get_component<Soft_body>(entity);

    // slime texture

    framebuffers[1]->bind();

    glViewport(0, 0, 64, 64);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    float size = 4.0f;

    vec2 avg_speed = vec2(0.0f);

    //

    std::vector<Tex_vertex> tris;

    vec2 min_v = vec2(FLT_MAX, FLT_MAX);
    vec2 max_v = vec2(-FLT_MAX, -FLT_MAX);
    for(auto p : soft_body.points) {
        min_v = min(min_v, p.position);
        max_v = max(max_v, p.position);
    }
    vec2 center = (min_v + max_v) * 0.5f;
    vec2 prev = center;
    center = (round(center * 16.0f)) / 16.0f;
    prev -= center;

    std::vector<vec2> offsets(soft_body.points.size(), vec2(0.0f));

    for(int i = 0; i < soft_body.points.size(); ++i) {
        vec2 a = soft_body.points[(i - 1 + soft_body.points.size()) % soft_body.points.size()].position;
        vec2 b = soft_body.points[i].position;
        vec2 c = soft_body.points[(i + 1) % soft_body.points.size()].position;

        vec2 normal_a = normalize(b - a);
        vec2 normal_b = normalize(c - b);

        normal_a = vec2(normal_a.y, -normal_a.x);
        normal_b = vec2(normal_b.y, -normal_b.x);

        vec2 n = normalize(normal_a + normal_b) * soft_body.inflate;

        offsets[i] = n;

        avg_speed += soft_body.points[i].velocity;
    }

    avg_speed /= float(soft_body.points.size());

    for(int i = 0; i < soft_body.points.size(); ++i) {
        vec2 a = soft_body.points[i].position + offsets[i];
        vec2 b = soft_body.points[(i + 1) % soft_body.points.size()].position + offsets[(i + 1) % soft_body.points.size()];

        a = a - center;//round((a - center) * 16.0f) / 16.0f;
        b = b - center;//round((b - center) * 16.0f) / 16.0f;

        vec2 aa = soft_body.target_ori * soft_body.targets[i];
        vec2 bb = soft_body.target_ori * soft_body.targets[(i + 1) % soft_body.points.size()];

        aa = (normalize(a) * 0.5f + 0.5f) * 24.0f;
        bb = (normalize(b) * 0.5f + 0.5f) * 24.0f;
        vec2 cc = vec2(0.5f) * 24.0f;

        Tex_vertex v;

        v.position = vec3(a, 0.5f);
        v.tex_coord = aa;
        tris.push_back(v);
        v.position = vec3(b, 0.5f);
        v.tex_coord = bb;
        tris.push_back(v);
        v.position = vec3(0.0f, 0.0f, 0.5f);
        v.tex_coord = cc;
        tris.push_back(v);
    }

    //

    float sc = 2.0f / size;

    mat4 view = scale(vec3(sc, sc, 1.0f));
    mat4 proj = identity<mat4>();
    mat4 model = identity<mat4>();

    core.shaders["texture"]->use();
    core.textures["slime"]->bind(0);

    vec3 color = vec3(1.0f);

    //

    vertices->vertex_buffer_data(tris.data(), tris.size(), sizeof(Object_vertex), GL_STREAM_DRAW);
    vertices->add_vertex_attribute(0, 3, GL_FLOAT, false, sizeof(float) * 5, 0);
    vertices->add_vertex_attribute(1, 2, GL_FLOAT, false, sizeof(float) * 5, sizeof(float) * 3);
    vertices->bind();

    glUniformMatrix4fv(0, 1, false, &proj[0][0]);
    glUniformMatrix4fv(1, 1, false, &view[0][0]);
    glUniformMatrix4fv(2, 1, false, &model[0][0]);
    glUniform4f(3, color.x, color.y, color.z,  1.0f);

    vertices->draw_vertices(GL_TRIANGLES);

    // slime render

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screen_size.x, screen_size.y);

    Transform2D& ct = ecs.get_component<Transform2D>(camera);
    Camera2D& cc = ecs.get_component<Camera2D>(camera);

    mat4 inv_rot = mat4(transpose(ct.orientation));

    view = inv_rot * scale(vec3(cc.scale, cc.scale, 1.0f)) * translate(vec3(-ct.position, 0.0f));

    float aspect_ratio = float(screen_size.y) / screen_size.x;
    proj = cc.proj;

    model = translate(vec3(center, 0.0f));

    core.shaders["sprite"]->use();

    /*
    layout(location = 0) uniform mat4 proj;
    layout(location = 1) uniform mat4 view;
    layout(location = 2) uniform mat4 model;
    layout(location = 3) uniform vec2 size;
    layout(location = 4) uniform vec2 offset;
    layout(location = 5) uniform vec4 range;
    layout(location = 6) uniform vec3 light;
    */

    framebuffers[1]->textures[0].bind(0);

    glUniformMatrix4fv(0, 1, false, &proj[0][0]);
    glUniformMatrix4fv(1, 1, false, &view[0][0]);
    glUniformMatrix4fv(2, 1, false, &model[0][0]);
    glUniform2f(3, 1.0f * size, 1.0f * size); // size
    glUniform2f(4, -0.5f * size, -0.5f * size); // offset
    glUniform4f(5, 0.0f, 0.0f, 1.0f, 1.0f); // range

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // render face

    std::vector<Tex_vertex> vs;
    vs = {
            Tex_vertex{vec3(0.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f)},
            Tex_vertex{vec3(1.0f, 0.0f, 0.0f), vec2(1.0f, 0.0f)},
            Tex_vertex{vec3(1.0f, 1.0f, 0.0f), vec2(1.0f, 1.0f)},
            Tex_vertex{vec3(0.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f)},
            Tex_vertex{vec3(1.0f, 1.0f, 0.0f), vec2(1.0f, 1.0f)},
            Tex_vertex{vec3(0.0f, 1.0f, 0.0f), vec2(0.0f, 1.0f)},
    };

    if(abs(avg_speed.x) > 1.5f) {
        look += core.delta_time * sign(avg_speed.x) * 5.0f;
        return_look = 0.25f;
    } else {
        return_look -= core.delta_time;
        if(return_look <= 0.0f) {
            float sg = sign(look);
            look -= core.delta_time * sg * 2.0f;
            if(sg != sign(look)) look = 0.0f;
        }
    }
    look = clamp(look, -1.0f, 1.0f);

    float width_b = ((max_v.x - min_v.x) * 0.5f - 17.0f / 16.0f * 0.5f - 0.0625f);
    width_b = max(width_b, 0.0f);

    vec2 face_size = vec2(17, 6);
    vec2 face_tex_pos = vec2(24, 0);

    for(Tex_vertex& tv : vs) {
        tv.position.x *= face_size.x / 16.0f;
        tv.position.y *= face_size.y / 16.0f;

        tv.tex_coord = tv.tex_coord * face_size + face_tex_pos;
    }

    vertices->vertex_buffer_data( vs.data(), vs.size(), sizeof(Object_vertex), GL_STREAM_DRAW);
    vertices->add_vertex_attribute(0, 3, GL_FLOAT, false, sizeof(float) * 5, 0);
    vertices->add_vertex_attribute(1, 2, GL_FLOAT, false, sizeof(float) * 5, sizeof(float) * 3);
    vertices->bind();

    vec2 face_pos = center - face_size / 16.0f * 0.5f;
    face_pos.x += look * width_b;

    face_pos = round(face_pos * 16.0f) / 16.0f;

    core.shaders["texture"]->use();
    core.textures["slime"]->bind(0);

    model = translate(vec3(face_pos, 0.0f));

    glUniformMatrix4fv(0, 1, false, &proj[0][0]);
    glUniformMatrix4fv(1, 1, false, &view[0][0]);
    glUniformMatrix4fv(2, 1, false, &model[0][0]);
    glUniform4f(3, color.x, color.y, color.z,  1.0f);

    vertices->draw_vertices(GL_TRIANGLES);
}


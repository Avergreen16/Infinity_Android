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

std::vector<Object_vertex> get_mesh(std::vector<vec2> v, vec2 radius, float border_width) {
    float sphere_segments = max(8, int32_t(8 * max(radius.x, radius.y)));

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

    std::vector<vec2> vv2;
    std::vector<vec2> norms;

    for(int i = 0 ; i < vv.size(); ++i) {
        vec2 v0 = vv[i];
        vec2 vprev = vv[(i - 1 + vv.size()) % vv.size()];
        vec2 vnext = vv[(i + 1 + vv.size()) % vv.size()];

        vec2 norm_prev = normalize(v0 - vprev);
        norm_prev = vec2(-norm_prev.y, norm_prev.x);

        vec2 norm_next = normalize(vnext - v0);
        norm_next = vec2(-norm_next.y, norm_next.x);

        vec2 n = normalize(norm_prev + norm_next);
        v0 += n * border_width;

        vv2.push_back(v0);
        norms.push_back(n);
    }

    std::vector<Object_vertex> vvv;
    for(int i = 0 ; i < vv.size(); ++i) {
        Object_vertex a;
        Object_vertex b;
        Object_vertex c;
        Object_vertex d;

        vec2 aa = vv[i];
        vec2 bb = vv[(i + 1) % vv.size()];
        vec2 cc = vv2[i];
        vec2 dd = vv2[(i + 1) % vv.size()];

        vec2 na = norms[i];
        vec2 nb = norms[(i + 1) % vv.size()];

        vec3 norm = vec3(normalize(na + nb), 1.0f);

        a.position = vec3(aa, 0.5f);
        b.position = vec3(bb, 0.5f);
        c.position = vec3(cc, 0.5f);
        d.position = vec3(dd, 0.5f);


        a.normal = norm;
        b.normal = norm;
        c.normal = norm;
        d.normal = norm;

        vvv.push_back(a);
        vvv.push_back(b);
        vvv.push_back(d);
        vvv.push_back(a);
        vvv.push_back(d);
        vvv.push_back(c);
    }

    for(int i = 0; i < vv2.size(); ++i) {
        vec3 v0 = vec3(vv2[i], 0.5f);
        vec3 v1 = vec3(vv2[(i + 1) % vv.size()], 0.5f);
        vec3 v2 = vec3(0.0f, 0.0f, 0.5f);

        vec3 vc = cross(v0 - v2, v1 - v2);
        if (vc.z < 0.0f) {
            vec3 temp = v1;
            v1 = v2;
            v2 = temp;
        }

        Object_vertex ov;
        ov.position = v0;
        ov.normal = vec3(0.0f, 0.0f, 1.0f);
        vvv.push_back(ov);

        ov.position = v1;
        vvv.push_back(ov);

        ov.position = v2;
        vvv.push_back(ov);
    }

    return vvv;
}

void create_mesh(Mesh& m, std::vector<vec2> v, vec2 radius, float border_width) {
    m.v_tris = std::shared_ptr<Vertices>(new Vertices);
    m.v_tris->init();

    m.border = border_width;

    auto vvv = get_mesh(v, radius, border_width);

    m.v_tris->vertex_buffer_data(vvv.data(), vvv.size(), sizeof(Object_vertex), GL_STATIC_DRAW);
    m.v_tris->add_vertex_attribute(0, 3, GL_FLOAT, false, sizeof(float) * 6, 0);
    m.v_tris->add_vertex_attribute(1, 3, GL_FLOAT, false, sizeof(float) * 6, sizeof(float) * 3);
}

void create_sprite(Sprite& sprite, std::vector<vec2> v, vec2 radius, float border_width) {
    sprite.border = border_width;

    auto vvv = get_mesh(v, radius, border_width);

    vec2 bb_min = vec2(FLT_MAX, FLT_MAX);
    vec2 bb_max = vec2(-FLT_MAX, -FLT_MAX);

    for(auto v : vvv) {
        bb_min = min(bb_min, vec2(v.position.x, v.position.y));
        bb_max = max(bb_max, vec2(v.position.x, v.position.y));
    }

    ivec2 size = ceil((bb_max - bb_min) * 16.0f);
    vec2 center = (bb_min + bb_max) * 0.5f;

    for(auto& v : vvv) v.position -= vec3(center, 0.0f);

    //

    Vertices vs;

    vs.vertex_buffer_data(vvv.data(), vvv.size(), sizeof(Object_vertex), GL_STATIC_DRAW);
    vs.add_vertex_attribute(0, 3, GL_FLOAT, false, sizeof(float) * 6, 0);
    vs.add_vertex_attribute(1, 3, GL_FLOAT, false, sizeof(float) * 6, sizeof(float) * 3);;

    std::shared_ptr<Framebuffer> fb0(new Framebuffer(size, {{{GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}, GL_COLOR_ATTACHMENT0, 0}, {{GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}, GL_COLOR_ATTACHMENT1, 1}}));

    fb0->bind();

    glViewport(0, 0, size.x, size.y);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    //

    mat4 identity = glm::identity<mat4>();
    mat4 scale = glm::scale(2.0f / vec3(float(size.x) / 16.0f, float(size.y) / 16.0f, 1.0f));

    core.shaders["shape_render"]->use();

    vs.bind();

    vec3 light = vec3(0.5f, -0.5f, 1.0f);

    glUniformMatrix4fv(0, 1, false, &identity[0][0]);
    glUniformMatrix4fv(1, 1, false, &scale[0][0]);
    glUniformMatrix4fv(2, 1, false, &identity[0][0]);
    glUniform4f(3, sprite.color.x, sprite.color.y, sprite.color.z, 1.0f);
    glUniform3fv(4, 1, &light[0]);

    vs.draw_vertices(GL_TRIANGLES);

    sprite.color_tex = std::make_shared<Texture>(std::move(fb0->textures[0]));
    sprite.normal_tex = std::make_shared<Texture>(std::move(fb0->textures[1]));
    sprite.range = vec4(0.0f, 0.0f, 1.0f, 1.0f);
    sprite.offset = -vec2(size) / 16.0f * 0.5f;
    sprite.size = vec2(size) / 16.0f;
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

    // framebuffers

    int width, height;
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);

    int pixel_size = 1;
    ivec2 fb_size = ivec2(width, height) / pixel_size;

    std::shared_ptr<Framebuffer> fb0(new Framebuffer(fb_size, {{{GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}, GL_COLOR_ATTACHMENT0, 0}}));
    framebuffers.push_back(fb0);
}

Renderer::Renderer() {
    Signature s = ecs.update_signature<Transform2D>();
    ecs.update_signature<Mesh>(s);
    collectors.push_back(Collector{s, false});

    s = ecs.update_signature<Transform2D>();
    ecs.update_signature<Sprite>(s);
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

    // calculate camera view
    vec2 ratio;
    if(width > height) ratio = vec2(1.0f, float(height) / width);
    else ratio = vec2(float(width) / height, 1.0f);

    Camera2D& camera_camera = ecs.get_component<Camera2D>(camera);
    camera_camera.proj = scale(vec3(1.0f / ratio, 1.0f));

    if(!vertices) {
        vertices = std::shared_ptr<Vertices>(new Vertices);
        vertices->init();
    }

    glDisable(GL_DEPTH_TEST);

    framebuffers[0]->bind();
    glViewport(0, 0, framebuffers[0]->size.x, framebuffers[0]->size.y);

    render_background();

    //render_text();

    for(uint32_t entity : collectors[0].entities) render_mesh(entity);
    for(uint32_t entity : collectors[1].entities) render_sprite(entity);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);

    //

    core.shaders["screen"]->use();
    framebuffers[0]->textures[0].bind(0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

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

    int width, height;
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);

    float aspect_ratio = float(height) / width;
    mat4 proj = cc.proj;

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
    mat4 proj = cc.proj;

    Transform2D& mesh_t = ecs.get_component<Transform2D>(entity);
    Mesh& mesh_m = ecs.get_component<Mesh>(entity);

    mat4 model = translate(vec3(mesh_t.position, 0.0f)) * mat4(mesh_t.orientation);

    core.shaders["shape"]->use();

    mesh_m.v_tris->bind();

    vec3 light = vec3(0.5f, -0.5f, 1.0f);

    glUniformMatrix4fv(0, 1, false, &proj[0][0]);
    glUniformMatrix4fv(1, 1, false, &view[0][0]);
    glUniformMatrix4fv(2, 1, false, &model[0][0]);
    glUniform4f(3, mesh_m.color.x, mesh_m.color.y, mesh_m.color.z, 1.0f);
    glUniform3fv(4, 1, &light[0]);

    mesh_m.v_tris->draw_vertices(GL_TRIANGLES);
}

void Renderer::render_sprite(uint32_t entity) {
    Transform2D& ct = ecs.get_component<Transform2D>(camera);
    Camera2D& cc = ecs.get_component<Camera2D>(camera);

    mat4 inv_rot = mat4(transpose(ct.orientation));

    mat4 view = inv_rot * scale(vec3(cc.scale, cc.scale, 1.0f)) * translate(vec3(-ct.position, 0.0f));

    int width, height;
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);

    float aspect_ratio = float(height) / width;
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


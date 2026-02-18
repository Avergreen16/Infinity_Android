//
// Created by plane on 2/13/2026.
//

#include "input.h"
#include "renderer.h"
#include "camera.h"
#include "gui.h"
#include "physics.h"

Input_system::Input_system() {

}

Input_system::~Input_system() {

}

void Input_system::init() {

}

void Input_system::call() {
    GUI_system& gui_system = ecs.get_system<GUI_system>();
    Physics_system& physics_system = ecs.get_system<Physics_system>();
    Renderer& renderer = ecs.get_system<Renderer>();

    int width, height;
    eglQuerySurface(renderer.display, renderer.surface, EGL_WIDTH, &width);
    eglQuerySurface(renderer.display, renderer.surface, EGL_HEIGHT, &height);

    vec2 ratio;
    if(width < height) ratio = vec2(1.0f, float(height) / width);
    else ratio = vec2(float(width) / height, 1.0f);

    Transform2D& camera_transform = ecs.get_component<Transform2D>(renderer.camera);
    Camera2D& camera_camera = ecs.get_component<Camera2D>(renderer.camera);

    if(gui_system.capture_id == 0xFFFFFFFF && held_pointer == 0xFFFFFFFF) {
        vec2 delta_motion = vec2(0.0f);

        if (pointers.size() >= 2) {
            auto it = pointers.begin();
            auto &pointer0 = it->second;
            ++it;
            auto &pointer1 = it->second;

            vec2 prev_avg = (pointer0.prev_pos + pointer1.prev_pos) * 0.5f;
            vec2 new_avg = (pointer0.pos + pointer1.pos) * 0.5f;

            delta_motion = new_avg - prev_avg;

            vec2 delta_0 = pointer0.pos - pointer0.prev_pos;
            vec2 diff_0 = pointer0.prev_pos - prev_avg;
            vec2 dir_0 = normalize(diff_0);

            vec2 delta_1 = pointer1.pos - pointer1.prev_pos;
            vec2 diff_1 = pointer1.prev_pos - prev_avg;
            vec2 dir_1 = normalize(diff_1);

            float zoom_0 = dot(dir_0, delta_0);
            vec2 rot_0 = delta_0 - dir_0 * zoom_0 - delta_motion;

            float zoom_1 = dot(dir_1, delta_1);
            vec2 rot_1 = delta_1 - dir_1 * zoom_1 - delta_motion;

            float delta_scale = 1.0f;
            float ds_0 = zoom_0 / length(diff_0) + 1.0f;
            float ds_1 = zoom_1 / length(diff_1) + 1.0f;

            delta_scale = (ds_0 + ds_1) * 0.5f;

            float delta_rotate = 0.0f;
            vec2 p0_prev = pointer0.prev_pos - prev_avg;
            vec2 p0_current = pointer0.pos - prev_avg;
            float angle0 = acos(clamp(dot(normalize(p0_prev), normalize(p0_current)), -1.0f, 1.0f));

            vec2 p1_prev = pointer1.prev_pos - prev_avg;
            vec2 p1_current = pointer1.pos - prev_avg;
            float angle1 = acos(clamp(dot(normalize(p1_prev), normalize(p1_current)), -1.0f, 1.0f));

            if (cross(vec3(p0_prev, 0.0f), vec3(p0_current, 0.0f)).z < 0.0) angle0 = -angle0;
            if (cross(vec3(p1_prev, 0.0f), vec3(p1_current, 0.0f)).z < 0.0) angle1 = -angle1;

            delta_rotate = (angle0 + angle1) * 0.5f;

            mat2 rot = glm::rotate(delta_rotate, vec3(0.0f, 0.0f, 1.0f));

            pivot = camera_transform.orientation *
                    (((prev_avg / vec2(width, height) * 2.0f - 1.0f)) * ratio /
                     camera_camera.scale) + camera_transform.position;

            camera_transform.position =
                    transpose(rot) * ((camera_transform.position - pivot) / delta_scale) + pivot;
            camera_transform.orientation = transpose(rot) * camera_transform.orientation;
            camera_camera.scale *= delta_scale;
        } else {
            for (auto &[id, pointer]: pointers) {
                delta_motion += pointer.pos - pointer.prev_pos;
            }

            if (pointers.size()) delta_motion /= float(pointers.size());
        }

        // change position

        vec2 pos = camera_transform.orientation * vec2(-delta_motion.x, -delta_motion.y);
        float ratio = min(width, height) * camera_camera.scale * 0.5f;
        pos /= ratio;

        camera_transform.position += pos;
    }

    for(auto& [id, pointer] : pointers) {
        pointer.prev_pos = pointer.pos;

        if(pointer.down == 0) {
            if(pointers.size() == 1 && held_pointer == 0xFFFFFFFF) {
                vec2 world_pos = camera_transform.orientation * (((pointer.pos / vec2(width, height) * 2.0f - 1.0f)) * ratio / camera_camera.scale) + camera_transform.position;

                for(uint32_t entity : physics_system.collectors[0].entities) {
                    Transform2D& tf = ecs.get_component<Transform2D>(entity);
                    Collider& c = ecs.get_component<Collider>(entity);

                    vec2 rel_point = transpose(tf.orientation) * (world_pos - tf.position);

                    if(!c.is_static && physics_system.collision_point(c, rel_point)) {
                        held_constraint = physics_system.constraints.size();
                        held_pointer = id;

                        Constraint constraint;
                        constraint.a = entity;

                        pos_constraint pc;
                        pc.a = rel_point;
                        pc.b = world_pos;
                        pc.vs = {vec2(1, 0), vec2(0, 1)};
                        pc.compliance = 0.00033;

                        constraint.pos.push_back(pc);

                        physics_system.constraints.push_back(constraint);

                        break;
                    }
                }
            }

            pointer.down = 1;
        } else if(pointer.down == 1) {
            pointer.down = 2;
        }
    }

    if(pointers.find(held_pointer) != pointers.end()) {
        Pointer& pointer = pointers[held_pointer];

        vec2 world_pos = camera_transform.orientation * (((pointer.pos / vec2(width, height) * 2.0f - 1.0f)) * ratio / camera_camera.scale) + camera_transform.position;

        Constraint& constraint = physics_system.constraints[held_constraint];

        constraint.pos[0].b = world_pos;
    } else {
        if(held_pointer != 0xFFFFFFFF) physics_system.constraints.erase(physics_system.constraints.begin() + held_constraint);
        held_pointer = 0xFFFFFFFF;
    }
}




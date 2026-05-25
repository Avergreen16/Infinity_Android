//
// Created by plane on 2/13/2026.
//

#include "input.h"
#include "renderer.h"
#include "camera.h"
#include "gui.h"
#include "physics.h"
#include "core.h"
#include "levels.h"

Input_system::Input_system() {

}

Input_system::~Input_system() {

}

void Input_system::init() {

}

void Input_system::call() {
    GUI_system &gui_system = ecs.get_system<GUI_system>();
    Physics_system &physics_system = ecs.get_system<Physics_system>();
    Renderer &renderer = ecs.get_system<Renderer>();

    int width, height;
    eglQuerySurface(renderer.display, renderer.surface, EGL_WIDTH, &width);
    eglQuerySurface(renderer.display, renderer.surface, EGL_HEIGHT, &height);
    int min_dimension = min(width, height);

    vec2 ratio;
    if (width > height) ratio = vec2(1.0f, float(height) / width);
    else ratio = vec2(float(width) / height, 1.0f);

    Transform2D &camera_transform = ecs.get_component<Transform2D>(renderer.camera);
    Camera2D &camera_camera = ecs.get_component<Camera2D>(renderer.camera);

    /*

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
        float ratio = max(width, height) * camera_camera.scale * 0.5f;
        pos /= ratio;

        camera_transform.position += pos;
    }
     */


    for (auto &[id, pointer]: pointers) {
        pointer.prev_pos = pointer.pos;

        if (pointer.down == 0) {
            /*
            if(pointers.size() == 1 && held_pointer == 0xFFFFFFFF) {
                vec2 world_pos = camera_transform.orientation * (((pointer.pos / vec2(width, height) * 2.0f - 1.0f)) * ratio / camera_camera.scale) + camera_transform.position;

                for(uint32_t entity : physics_system.collectors[0].entities) {
                    Transform2D& tf = ecs.get_component<Transform2D>(entity);
                    Collider& c = ecs.get_component<Collider>(entity);

                    vec2 rel_point = transpose(tf.orientation) * (world_pos - tf.position);

                    if(!c.is_static && physics_system.collision_point(c, vec2(0.0f), rel_point)) {
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
            */

            pointer.down = 1;
        } else if (pointer.down == 1) {
            pointer.down = 2;
        }
    }

    /*
    if(pointers.find(held_pointer) != pointers.end()) {
        Pointer& pointer = pointers[held_pointer];

        vec2 world_pos = camera_transform.orientation * (((pointer.pos / vec2(width, height) * 2.0f - 1.0f)) * ratio / camera_camera.scale) + camera_transform.position;

        Constraint& constraint = physics_system.constraints[held_constraint];

        constraint.pos[0].b = world_pos;
    } else {
        if(held_pointer != 0xFFFFFFFF) physics_system.constraints.erase(physics_system.constraints.begin() + held_constraint);
        held_pointer = 0xFFFFFFFF;
    }
    */

    bool do_jump = false;
    bool do_jump_cooldown = false;
    float do_jump_value = 0.0f;

    vec2 center = vec2(width - float(min_dimension) / 8.0f * 1.5f,
                       float(min_dimension) / 8.0f * 1.5f);
    float radius = float(min_dimension) / 8.0f * 0.625f;

    if (jump_pointer != 0xFFFFFFFF) {
        if (pointers.find(jump_pointer) == pointers.end()) {
            jump_pointer = 0xFFFFFFFF;
            if (jump != 0.0f) {
                do_jump = true;
                do_jump_cooldown = true;
                do_jump = clamp(jump, 0.0f, 1.0f);

                jump = min(jump, 1.0f);
            }
        } else {
            jump += core.delta_time / jump_charge;
        }
    } else {
        if (jump > 0.0f) {
            jump -= core.delta_time / jump_cooldown;

            do_jump_cooldown = true;
        } else jump = 0.0f;
    }

    if (jump == 0.0f) {
        if (pointers.size() && jump_pointer == 0xFFFFFFFF) {
            for (auto &[id, p]: pointers) {
                if (p.down == 1) {
                    if (length(p.pos - center) < radius) {
                        jump_pointer = id;
                    }
                }
            }
        }
    }




    // joystick
    center = vec2(float(min_dimension) / 8.0f * 1.5f);
    radius = float(min_dimension) / 8.0f;

    joystick = vec2(0.0f);

    if (joystick_pointer != 0xFFFFFFFF) {
        if (pointers.find(joystick_pointer) == pointers.end()) {
            joystick_pointer = 0xFFFFFFFF;
        } else {
            Pointer &p = pointers[joystick_pointer];

            vec2 rel_pos = p.pos - center;
            rel_pos /= radius;

            if (length(rel_pos) > 1.0f) rel_pos = normalize(rel_pos);

            joystick = rel_pos;
        }
    }

    if (pointers.size() && joystick_pointer == 0xFFFFFFFF) {
        for (auto &[id, p]: pointers) {
            if (p.down == 1) {
                if (length(p.pos - center) < radius) {
                    joystick_pointer = id;
                }
            }
        }
    }

    Soft_body& player_soft_body = ecs.get_component<Soft_body>(player_entity);
    //Collider &player_collider = ecs.get_component<Collider>(player_entity);
    //Transform2D &player_transform = ecs.get_component<Transform2D>(player_entity);

    Physics_system &ps = ecs.get_system<Physics_system>();

    // stick
    bool move = length(joystick) != 0.0f;

    /*


    struct ppoint {
        uint32_t i;

        vec2 pa = vec2(0.0f);
        vec2 pb = vec2(0.0f);
        vec2 normal = vec2(0.0f);
        uint32_t n = 0;

        vec2 rel;
    };

    std::vector<ppoint> points;

    float rad = player_collider.shapes[0].radius.x;

    std::vector<Collision_data> collisions = ps.collide(player_transform, {vec2(0.0f)}, rad * 1.125f);

    slime_constraints.clear();
    std::vector<vec2> ns;

    Renderer &r = ecs.get_system<Renderer>();
    //r.points.clear();
    //r.normals.clear();
    //r.cs.clear();

    for (Collision_data &d: collisions) {
        vec2 va = normalize(d.pb) * rad;
        vec2 vb = d.pa;

        bool insert = true;

        vec2 global_vb = vb;
        if (d.a != 0xFFFFFFFF) {
            Transform2D &tf = ecs.get_component<Transform2D>(d.a);

            global_vb = tf.orientation * vb + tf.position;
        }

        vec2 rel_vb = global_vb - player_transform.position;

        //r.points.push_back(global_vb);
        //r.normals.push_back(d.normal);

        uint32_t ii = 0;

        for (ppoint &pp: points) {
            if (pp.i == d.a) {
                if (length(pp.rel - rel_vb) < 0.0625f) {
                    if (dot(normalize(pp.rel), pp.normal) < dot(normalize(rel_vb), d.normal)) {
                        //r.cs[pp.n] = vec3(1.0f, 0.0f, 0.0f);
                        pp.pb = vb;
                        pp.pa = va;
                        pp.normal = d.normal;
                        pp.rel = rel_vb;
                        pp.n = ii;

                        r.cs.push_back(vec3(0.0f, 1.0f, 0.0f));
                    } else {
                        r.cs.push_back(vec3(1.0f, 0.0f, 0.0f));
                    }

                    insert = false;
                    break;
                } else {
                    float da = dot(rel_vb - pp.rel, pp.normal);
                    float db = dot(pp.rel - rel_vb, d.normal);
                    if (da >= -0.0001f && length(pp.rel) < length(rel_vb)) {
                        insert = false;

                        r.cs.push_back(vec3(0.0f, 0.0f, 1.0f));

                        break;
                    } else if (db >= -0.0001f && length(pp.rel) > length(rel_vb)) {
                        //r.cs[pp.n] = vec3(0.0f, 0.0f, 1.0f);
                        pp.pb = vb;
                        pp.pa = va;
                        pp.normal = d.normal;
                        pp.rel = rel_vb;
                        pp.n = ii;

                        r.cs.push_back(vec3(0.0f, 1.0f, 0.0f));

                        insert = false;
                        break;
                    }
                }
            }

            ++ii;
        }

        if (insert) {
            ppoint new_pp;
            new_pp.n = points.size();
            new_pp.pa = va;
            new_pp.pb = vb;
            new_pp.normal = d.normal;

            new_pp.i = d.a;

            new_pp.rel = rel_vb;

            points.push_back(new_pp);

            r.cs.push_back(vec3(0.0f, 1.0f, 0.0f));
        }

        //r.points.push_back(player_transform.orientation * va + player_transform.position);
        //r.normals.push_back(d.normal);
        r.cs.push_back(vec3(1.0f, 1.0f, 0.25f));
    }

    //LOGD("%i, %i", points.size(), collisions.size());

    for (ppoint pp: points) {
        //r.points.push_back(pp.rel + player_transform.position);
        //r.normals.push_back(pp.normal);
        //r.cs.push_back(true);

        Constraint constraint;

        constraint.a = player_entity;
        constraint.b = pp.i;


        pos_constraint c;
        c.a = pp.pa;
        c.b = pp.pb;
        if(move && jump == 0.0f) c.vs = {normalize(pp.normal)};
        else {
            c.vs = {normalize(pp.normal), normalize(vec2(-pp.normal.y, pp.normal.x))};

            rot_constraint r;
            r.a = transpose(player_transform.orientation) * vec2(0.0f, 1.0f);
            if(pp.i == NULL_ENTITY) {
                r.b = vec2(0.0f, 1.0f);
            } else {
                Transform2D& ta = ecs.get_component<Transform2D>(pp.i);
                r.b = transpose(ta.orientation) * vec2(0.0f, 1.0f);
            }

            constraint.rot = {r};
        }

        //c.vs = {normalize(vec2(0.0f, 1.0f))};


        c.limit = 3.0f;

        constraint.pos = {c};
        ns.push_back(normalize(pp.normal));

        //slime_constraints.push_back(constraint);
    }

    */

    //if(player_collider.colliding_with.size()) player_collider.allow_gravity = false;
    //else player_collider.allow_gravity = true;


    // movement
    vec2 up = vec2(0.0f, 1.0f);

    /*
    if (move) {
        float mm = FLT_MAX;
        uint32_t i = 0;
        uint32_t id = 0xFFFFFFFF;
        for (vec2 n: ns) {
            float m = abs(dot(n, joystick));

            if (mm > m) {
                up = n;
                mm = m;

                id = i;
            }
            ++i;
        }

        if (id != 0xFFFFFFFF) {
            slime_constraints = {slime_constraints[id]};
        }
    }
     */

    //player_collider.angular_velocity = 0.0f;

    {
        vec2 avg_velocity = vec2(0.0f);
        for(auto& p : player_soft_body.points) avg_velocity += p.velocity;
        avg_velocity /= float(player_soft_body.points.size());

        vec2 target_velocity = joystick;
        //if (jump != 0.0f) target_velocity = vec2(0.0f);

        target_velocity *= 8.0f;
        target_velocity = vec2(target_velocity.x, 0.0f);

        /*if(slime_constraints.size()) {
            if(!(slime_constraints[0].b == NULL_ENTITY)) {
                Transform2D& ta = ecs.get_component<Transform2D>(slime_constraints[0].b);
                Collider& ca = ecs.get_component<Collider>(slime_constraints[0].b);

                vec2 vp = ta.orientation * slime_constraints[0].pos[0].b;

                vec2 add_v = Physics_system::calculate_point_velocity(&ca, vp);
                target_velocity += add_v;
            }
        }*/

        vec2 diff = target_velocity - avg_velocity;
        diff = diff - up * dot(up, diff);

        vec2 delta = diff;
        if (length(delta) > core.delta_time * 65.0f) delta *= core.delta_time * 65.0f /
                                                                 length(delta);

        for(auto& p : player_soft_body.points) p.velocity += delta;
    }

    //

    if(do_jump && length(joystick) != 0.0f) {
        slime_constraints.clear();

        //player_collider.velocity += normalize(joystick) * 16.0f;
    }

    if(do_jump_cooldown) slime_constraints.clear();
}




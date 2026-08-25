//
// Created by plane on 2/14/2026.
//

#include "gui.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "physics.h"

std::string numbers = "0123456789\x80\x81\x82\x83\x84\x85";

float italic_factor = 1.0f / 3.5f;
float bold_factor = 1.0f;

std::string to_base(float f, uint32_t base, uint32_t num_figures) {
    bool is_neg = f < 0.0f;
    f = abs(f);

    uint64_t integer_part = floor(f);
    f = fract(f);

    std::string integer_text;
    do {
        uint64_t ii = integer_part % base;
        integer_part = integer_part / base;
        integer_text.push_back(numbers[ii]);
    } while(integer_part != 0);

    std::string mantissa_text;
    uint64_t is_zeroes = 0;
    for(int j = 0; j < num_figures; ++j) {
        f *= base;
        uint32_t ii = floor(f);
        if(ii) is_zeroes = j + 1;

        mantissa_text += numbers[ii];

        f = fract(f);
    }

    std::reverse(integer_text.begin(), integer_text.end());

    std::string result;
    if(is_neg) result = "-";

    result += integer_text;

    if(is_zeroes) result += "." + std::string(mantissa_text.begin(), mantissa_text.begin() + is_zeroes);

    return result + "\x86";
}

GUI_system::GUI_system() {

}

GUI_system::~GUI_system() {

}

void GUI_system::init() {
    if(fonts.size() == 0) {
        fonts.emplace("default_mono", Font("fonts/alter_mono.afont"));
    }
}

void GUI_system::ui_loop() {
    Renderer& renderer = ecs.get_system<Renderer>();
    Physics_system& ps = ecs.get_system<Physics_system>();
    Input_system& input_system = ecs.get_system<Input_system>();

    if(input_system.pointers.find(capture_id) == input_system.pointers.end()) {
        capture_id = 0xFFFFFFFF;
    }

    Transform2D& ct = ecs.get_component<Transform2D>(renderer.camera);

    //

    int width, height;
    eglQuerySurface(renderer.display, renderer.surface, EGL_WIDTH, &width);
    eglQuerySurface(renderer.display, renderer.surface, EGL_HEIGHT, &height);

    static bool open_settings = false;
    static bool open_chat = false;
    static bool open_stats = false;

    //

    set_buffer(vec2(0.0f, 96.0f));
    set_mode(PM_TOP_LEFT);
    column();

    std::function<void()> lambda = []() {
        open_settings = !open_settings;
    };
    if(open_settings) {
        button({160.0f, 96.0f}, {54, 54, 64, 64}, vec4(1.0f, 1.0f, 1.0f, 0.25f), lambda);
    } else {
        button({96.0f, 96.0f}, {54, 54, 64, 64}, vec4(1.0f, 1.0f, 1.0f, 0.25f), lambda);
    }

    lambda = []() {
        open_stats = !open_stats;
    };
    if(open_stats) {
        button({160.0f, 96.0f}, {54, 34, 64, 44}, vec4(1.0f, 1.0f, 1.0f, 0.25f), lambda);
    } else {
        button({96.0f, 96.0f}, {54, 34, 64, 44}, vec4(1.0f, 1.0f, 1.0f, 0.25f), lambda);
    }

    lambda = []() {
        open_chat = !open_chat;
    };
    if(open_chat) {
        button({160.0f, 96.0f}, {54, 44, 64, 54}, vec4(1.0f, 1.0f, 1.0f, 0.25f), lambda);
    } else {
        button({96.0f, 96.0f}, {54, 44, 64, 54}, vec4(1.0f, 1.0f, 1.0f, 0.25f), lambda);
    }

    reset();
    set_mode(PM_TOP_RIGHT);
    column();

    lambda = [&ps]() {
        ps.sim_active = !ps.sim_active;
    };
    if(ps.sim_active) {
        button({96.0f, 96.0f}, {54, 24, 64, 34}, vec4(1.0f, 1.0f, 1.0f, 0.25), lambda);
    } else {
        button({96.0f, 96.0f}, {54, 14, 64, 24}, vec4(1.0f, 1.0f, 1.0f, 0.25f), lambda);
    }

    if(open_settings) {
        Window_state ws;
        ws.size = {512.0f, 768.0f};
        ws.position = {width * 0.5f - ws.size.x * 0.5f, height * 0.5f - ws.size.y * 0.5f};
        ws.label = "settings";

        //

        lambda = []() {
            open_settings = false;
        };
        window("settings_window", ws, lambda);

        set_buffer(vec2(8, 8));
        set_mode(PM_TOP_LEFT);

        row();
        column();

        set_mode(PM_CENTER);
        debug_square(vec2(32, 32), vec4(1.0f, 0.25f, 0.25f, 1.0f));
        debug_square(vec2(64, 64), vec4(1.0f, 0.625f, 0.25f, 1.0f));
        debug_square(vec2(96, 96), vec4(1.0f, 1.0f, 0.25f, 1.0f));
        debug_square(vec2(64, 64), vec4(1.0f, 0.625f, 0.25f, 1.0f), true);
    }

    if(open_stats) {
        Window_state ws;
        ws.size = {512.0f, 768.0f};
        ws.position = {width * 0.5f - ws.size.x * 0.5f, height * 0.5f - ws.size.y * 0.5f};
        ws.label = "stats";

        //
        lambda = []() {
            open_stats = false;
        };
        window("stats_window", ws, lambda);

        //std::string pos_text = to_base(ct.position.x, 16, 3) + " " + to_base(ct.position.y, 16, 3);
        //text(pos_text);
    }

    if(open_chat) {
        Window_state ws;
        ws.size = {512.0f, 768.0f};
        ws.position = {width * 0.5f - ws.size.x * 0.5f, height * 0.5f - ws.size.y * 0.5f};
        ws.label = "chat";

        //

        lambda = []() {
            open_chat = false;
        };
        window("chat_window", ws, lambda);
    }
}

vec4 clip(vec4 a, vec4 b) {
    return vec4(max(a.x, b.x), max(a.y, b.y), min(a.z, b.z), min(a.w, b.w));
}

/*
 *
 */

void GUI_system::call() {
    vertices.clear();
    widgets.clear();

    for(auto& [k, state] : window_state) state.active = false;

    //
    Renderer& renderer = ecs.get_system<Renderer>();

    int width, height;
    eglQuerySurface(renderer.display, renderer.surface, EGL_WIDTH, &width);
    eglQuerySurface(renderer.display, renderer.surface, EGL_HEIGHT, &height);

    Widget start_widget;
    start_widget.position = vec2(0.0f);
    start_widget.size = vec2(width, height);

    widgets = {start_widget};
    widget_path = {0};

    ui_loop();

    // widget measure

    std::vector<uint32_t> path = {0};
    std::vector<uint32_t> child_ids = {0};

    while(true) {
        if (path.size() == 0) break;

        Widget &w = widgets[path.back()];

        vec2 children_size = vec2(0.0f);

        if (w.children.size() <= child_ids.back()) {
            // process children positions
            if (w.layout_mode == LM_ROW) {
                for (int i = 0; i < w.children.size(); ++i) {
                    Widget &wchild = widgets[w.children[i]];

                    children_size.x += wchild.size.x;
                    if(i != w.children.size() - 1) children_size.x += w.buffer.x;

                    children_size.y = max(children_size.y, wchild.size.y);
                }
            } else if (w.layout_mode == LM_COLUMN) {
                for (int i = 0; i < w.children.size(); ++i) {
                    Widget &wchild = widgets[w.children[i]];

                    children_size.y += wchild.size.y;
                    if(i != 0 && i != w.children.size() - 1) children_size.y + w.buffer.y;

                    children_size.x = max(children_size.x, wchild.size.x);
                }
            } else if (w.layout_mode == LM_GRID) {
                if(w.data[0] == 0) {
                    int num_rows = w.data[1];

                } else {
                    int num_columns = w.data[0];

                    vec2 rel_cursor = vec2(0.0f);

                    std::vector<float> rows;
                    std::vector<float> columns;

                    for(int i = 0; i < w.children.size(); ++i) {
                        ivec2 pos = {i % num_columns, i / num_columns};

                        if(rows.size() < pos.y + 1) rows.resize(pos.y + 1, 0.0f);
                        if(columns.size() < pos.x + 1) columns.resize(pos.x + 1, 0.0f);

                        Widget &wchild = widgets[w.children[i]];

                        rows[pos.y] = max(rows[pos.y], wchild.size.y);
                        columns[pos.x] = max(columns[pos.x], wchild.size.x);
                    }

                    for(int i = 0; i < w.children.size(); ++i) {
                        ivec2 pos = {i % num_columns, i / num_columns};

                        Widget &wchild = widgets[w.children[i]];

                        for(int i = 0; i < pos.x; ++i) {
                            children_size.x += columns[i];
                            if(i != pos.y - 1) children_size.x += w.buffer.x;
                        }
                        for(int i = 0; i < pos.y; ++i) {
                            children_size.y += rows[i];
                            if(i != pos.y - 1) children_size.y += w.buffer.y;
                        }
                    }
                }
            } else if(w.layout_mode == LM_VOID) {

            }

            // process self size
            if(w.size_mode == SM_SURROUND) {
                w.size = vec2(0.0f);

                for(uint32_t child : w.children) {
                    Widget& wchild = widgets[child];

                    w.size = children_size;
                }
            }

            // go up
            path.pop_back();
            child_ids.pop_back();
        } else {
            path.push_back(w.children[child_ids.back()]);

            ++child_ids.back();
            child_ids.push_back(0);
        }
    }

    // widget solving
    path = {0};
    child_ids = {0};

    while(true) {
        if (path.size() == 0) break;

        Widget &w = widgets[path.back()];

        if (w.children.size() <= child_ids.back()) {
            float bounds = 0.0f;
            float buffer_subtract = 0.0f;

            // get bounds and buffer
            uint32_t current = path.back();
            while(true) {
                Widget& wchild = widgets[current];

                if(wchild.size_mode == SM_STATIC) {
                    bounds = wchild.size.x;
                    break;
                } else {
                    buffer_subtract += wchild.buffer.x;
                    if(wchild.parent != NULL_ENTITY) {
                        current = wchild.parent;
                    } else break;
                }
            }

            // get available space
            float available_space = bounds - buffer_subtract * 2.0f;

            if (w.layout_mode == LM_ROW) {
                if(widgets[w.parent].size_mode == SM_STATIC) {
                    LOGD("%f", available_space);

                    std::vector<uint32_t> v_expand;
                    std::vector<uint32_t> vs;

                    for (int i = 0; i < w.children.size(); ++i) {
                        Widget &wchild = widgets[w.children[i]];

                        if (wchild.size_mode == SM_FILL) {
                            v_expand.push_back(w.children[i]);
                            vs.push_back(w.children[i]);
                        } else if (wchild.size_mode == SM_SURROUND) {
                            for (int j = 0; j < wchild.children.size(); ++j) {
                                Widget &wchild2 = widgets[wchild.children[j]];

                                if (wchild2.size_mode == SM_FILL) {
                                    v_expand.push_back(wchild.children[j]);
                                    vs.push_back(w.children[i]);
                                }
                            }
                        }

                        available_space -= wchild.size.x;
                        if (i != w.children.size() - 1) available_space -= w.buffer.x;
                    }

                    std::set<uint32_t> n(vs.begin(), vs.end());

                    float add = available_space / n.size();

                    for (int i = 0; i < v_expand.size(); ++i) {
                        Widget &wchild = widgets[v_expand[i]];

                        if (wchild.size_mode == SM_FILL) {
                            if(v_expand[i] == vs[i]) {
                                wchild.size.x += add;
                            } else {
                                Widget& wparent = widgets[vs[i]];

                                wchild.size.x = max(wchild.size.x, wparent.size.x);
                                wchild.size.x += add;
                                wchild.size.x = max(wchild.size.x, wparent.size.x);
                            }
                        }
                    }
                }
            } else if (w.layout_mode == LM_COLUMN) {
                if(widgets[w.parent].size_mode == SM_STATIC) {
                    std::vector<uint32_t> v_expand;

                    for (int i = 0; i < w.children.size(); ++i) {
                        Widget &wchild = widgets[w.children[i]];

                        if (wchild.size_mode == SM_FILL) v_expand.push_back(w.children[i]);
                    }

                    float add = available_space;

                    for (uint32_t id: v_expand) {
                        Widget &wchild = widgets[id];

                        if (wchild.size_mode == SM_FILL) {
                            wchild.size.x = max(wchild.size.x, w.size.x);
                            wchild.size.x += add;
                            wchild.size.x = max(wchild.size.x, w.size.x);
                        }
                    }
                }
            } else if (w.layout_mode == LM_GRID) {
                if(widgets[w.parent].size_mode == SM_STATIC) {
                    int num_columns = w.data[0];

                    std::vector<float> columns;
                    std::vector<bool> has_fill;
                    uint32_t num_f = 0;

                    for (int i = 0; i < w.children.size(); ++i) {
                        ivec2 pos = {i % num_columns, i / num_columns};

                        if (columns.size() < pos.x + 1) {
                            columns.resize(pos.x + 1, 0.0f);
                            has_fill.resize(pos.x + 1, false);
                        }

                        Widget &wchild = widgets[w.children[i]];

                        columns[pos.x] = max(columns[pos.x], wchild.size.x);
                        if (wchild.size_mode == SM_FILL) {
                            if (has_fill[pos.x] == false) ++num_f;
                            has_fill[pos.x] = true;
                        }
                    }

                    for (int i = 0; i < columns.size(); ++i) {
                        float c = columns[i];

                        available_space -= c;
                        if (i != columns.size() - 1) available_space -= w.buffer.x;
                    }
                    float add = available_space / num_f;

                    for (int i = 0; i < w.children.size(); ++i) {
                        ivec2 pos = {i % num_columns, i / num_columns};
                        Widget &wchild = widgets[w.children[i]];

                        if (wchild.size_mode == SM_FILL) {
                            wchild.size.x = max(wchild.size.x, columns[pos.x]);
                            wchild.size.x += add;
                            wchild.size.x = max(wchild.size.x, columns[pos.x]);
                        }
                    }
                }
            } else if(w.layout_mode == LM_VOID) {

            }

            // go up
            path.pop_back();
            child_ids.pop_back();
        } else {
            path.push_back(w.children[child_ids.back()]);

            ++child_ids.back();
            child_ids.push_back(0);
        }
    }

    for(Widget& w : widgets) w.size_func(w);

    // widget relative placing
    path = {0};
    child_ids = {0};

    while(true) {
        if (path.size() == 0) break;

        Widget &w = widgets[path.back()];

        if (w.children.size() <= child_ids.back()) {
            // process children positions
            if (w.layout_mode == LM_ROW) {
                vec2 rel_cursor = w.child_offset;

                float max_size = 0.0f;
                bool first = true;

                for (uint32_t child: w.children) {
                    Widget &wchild = widgets[child];

                    wchild.position = rel_cursor;

                    rel_cursor.x += wchild.size.x + w.buffer.x;

                    max_size = max(wchild.size.y, max_size);
                }

                for(uint32_t child: w.children) {
                    Widget &wchild = widgets[child];

                    if(wchild.position_mode == PM_TOP_LEFT || wchild.position_mode == PM_TOP_CENTER || wchild.position_mode == PM_TOP_RIGHT) {
                        wchild.position.y = max_size - wchild.size.y;
                    } else if(wchild.position_mode == PM_CENTER_LEFT || wchild.position_mode == PM_CENTER || wchild.position_mode == PM_CENTER_RIGHT) {
                        wchild.position.y = (max_size - wchild.size.y) * 0.5f;
                    } else if(wchild.position_mode == PM_BOTTOM_LEFT || wchild.position_mode == PM_BOTTOM_CENTER || wchild.position_mode == PM_BOTTOM_RIGHT) {
                        wchild.position.y = 0.0f;
                    }
                }
            } else if (w.layout_mode == LM_COLUMN) {
                vec2 rel_cursor = w.child_offset;

                float max_size = 0.0f;
                bool first = true;

                for (uint32_t child: w.children) {
                    Widget &wchild = widgets[child];

                    wchild.position = rel_cursor;
                    rel_cursor.y += wchild.size.y + w.buffer.y;

                    max_size = max(wchild.size.x, max_size);
                }

                for(uint32_t child: w.children) {
                    Widget &wchild = widgets[child];

                    if(wchild.position_mode == PM_TOP_LEFT || wchild.position_mode == PM_CENTER_LEFT || wchild.position_mode == PM_BOTTOM_LEFT) {
                        wchild.position.x = 0.0f;
                    } else if(wchild.position_mode == PM_TOP_CENTER || wchild.position_mode == PM_CENTER || wchild.position_mode == PM_BOTTOM_CENTER) {
                        wchild.position.x = (max_size - wchild.size.x) * 0.5f;
                    } else if(wchild.position_mode == PM_TOP_RIGHT || wchild.position_mode == PM_CENTER_RIGHT || wchild.position_mode == PM_BOTTOM_RIGHT) {
                        wchild.position.x = max_size - wchild.size.x;
                    }
                }
            } else if (w.layout_mode == LM_GRID) {
                if(w.data[0] == 0) {
                    int num_rows = w.data[1];

                } else {
                    int num_columns = w.data[0];

                    vec2 rel_cursor = w.child_offset;

                    std::vector<float> rows;
                    std::vector<float> columns;

                    for(int i = 0; i < w.children.size(); ++i) {
                        ivec2 pos = {i % num_columns, i / num_columns};

                        if(rows.size() < pos.y + 1) rows.resize(pos.y + 1, 0.0f);
                        if(columns.size() < pos.x + 1) columns.resize(pos.x + 1, 0.0f);

                        Widget &wchild = widgets[w.children[i]];

                        rows[pos.y] = max(rows[pos.y], wchild.size.y);
                        columns[pos.x] = max(columns[pos.x], wchild.size.x);
                    }

                    for(int i = 0; i < w.children.size(); ++i) {
                        ivec2 pos = {i % num_columns, i / num_columns};

                        Widget &wchild = widgets[w.children[i]];

                        vec2 origin = vec2(0.0f);
                        vec2 box = vec2(0.0f);

                        for(int i = 0; i < pos.x; ++i) {
                            origin.x += columns[i] + w.buffer.x;
                        }
                        for(int i = 0; i < pos.y; ++i) {
                            origin.y += rows[i] + w.buffer.y;
                        }

                        box = vec2(columns[pos.x], rows[pos.y]);

                        if(wchild.position_mode == PM_TOP_LEFT || wchild.position_mode == PM_TOP_CENTER || wchild.position_mode == PM_TOP_RIGHT) {
                            wchild.position.y = box.y - wchild.size.y;
                        } else if(wchild.position_mode == PM_CENTER_LEFT || wchild.position_mode == PM_CENTER || wchild.position_mode == PM_CENTER_RIGHT) {
                            wchild.position.y = (box.y - wchild.size.y) * 0.5f;
                        } else if(wchild.position_mode == PM_BOTTOM_LEFT || wchild.position_mode == PM_BOTTOM_CENTER || wchild.position_mode == PM_BOTTOM_RIGHT) {
                            wchild.position.y = 0.0f;
                        }

                        if(wchild.position_mode == PM_TOP_LEFT || wchild.position_mode == PM_CENTER_LEFT || wchild.position_mode == PM_BOTTOM_LEFT) {
                            wchild.position.x = 0.0f;
                        } else if(wchild.position_mode == PM_TOP_CENTER || wchild.position_mode == PM_CENTER || wchild.position_mode == PM_BOTTOM_CENTER) {
                            wchild.position.x = (box.x - wchild.size.x) * 0.5f;
                        } else if(wchild.position_mode == PM_TOP_RIGHT || wchild.position_mode == PM_CENTER_RIGHT || wchild.position_mode == PM_BOTTOM_RIGHT) {
                            wchild.position.x = box.x - wchild.size.x;
                        }

                        wchild.position += origin;
                        wchild.position += rel_cursor;
                    }
                }
            } else if(w.layout_mode == LM_VOID) {
                for(uint32_t child : w.children) {
                    Widget &wchild = widgets[child];

                    if(wchild.position_mode == PM_TOP_LEFT || wchild.position_mode == PM_CENTER_LEFT || wchild.position_mode == PM_BOTTOM_LEFT) {
                        wchild.position.x = wchild.buffer.x;
                    } else if(wchild.position_mode == PM_TOP_CENTER || wchild.position_mode == PM_CENTER || wchild.position_mode == PM_BOTTOM_CENTER) {
                        wchild.position.x = (w.size.x - wchild.size.x) * 0.5f;
                    } else if(wchild.position_mode == PM_TOP_RIGHT || wchild.position_mode == PM_CENTER_RIGHT || wchild.position_mode == PM_BOTTOM_RIGHT) {
                        wchild.position.x = w.size.x - wchild.size.x - wchild.buffer.x;
                    }

                    if(wchild.position_mode == PM_TOP_LEFT || wchild.position_mode == PM_TOP_CENTER || wchild.position_mode == PM_TOP_RIGHT) {
                        wchild.position.y = w.size.y - wchild.size.y - wchild.buffer.y;
                    } else if(wchild.position_mode == PM_CENTER_LEFT || wchild.position_mode == PM_CENTER || wchild.position_mode == PM_CENTER_RIGHT) {
                        wchild.position.y = (w.size.y - wchild.size.y) * 0.5f;
                    } else if(wchild.position_mode == PM_BOTTOM_LEFT || wchild.position_mode == PM_BOTTOM_CENTER || wchild.position_mode == PM_BOTTOM_RIGHT) {
                        wchild.position.y = wchild.buffer.y;
                    }
                }
            }

            // process self size
            if(w.size_mode == SM_SURROUND) {
                w.size = vec2(0.0f);

                for(uint32_t child : w.children) {
                    Widget& wchild = widgets[child];
                    vec2 max_ = wchild.position + wchild.size;

                    w.size = max(max_, w.size);
                }
            }

            if(w.size_mode == SM_STATIC) {
                vec2 size = vec2(0.0f);
                for(uint32_t child : w.children) {
                    Widget& wchild = widgets[child];
                    float max_y = 0.0f;

                    vec2 max_ = wchild.size + wchild.buffer;

                    size = max(max_, size);

                    if(wchild.position_mode == PM_TOP_LEFT || wchild.position_mode == PM_TOP_CENTER || wchild.position_mode == PM_TOP_RIGHT) {
                        w.data.push_back(0);
                    } else if(wchild.position_mode == PM_CENTER_LEFT || wchild.position_mode == PM_CENTER || wchild.position_mode == PM_CENTER_RIGHT) {
                        w.data.push_back(1);
                    } else if(wchild.position_mode == PM_BOTTOM_LEFT || wchild.position_mode == PM_BOTTOM_CENTER || wchild.position_mode == PM_BOTTOM_RIGHT) {
                        w.data.push_back(2);
                    }
                }

                w.data.push_back(size.y);
            }

            // go up
            path.pop_back();
            child_ids.pop_back();
        } else {
            path.push_back(w.children[child_ids.back()]);

            ++child_ids.back();
            child_ids.push_back(0);
        }
    }


    // widget absolute placement

    std::vector<vec2> new_positions(widgets.size());

    for(uint32_t i = 0; i < widgets.size(); ++i) {
        vec2 pos = vec2(0.0f);

        uint32_t current = i;

        while(true) {
            Widget& w = widgets[current];

            pos += w.position;

            if(w.parent == NULL_ENTITY || w.position_mode == PM_STATIC) break;
            else current = w.parent;
        }

        new_positions[i] = pos;
    }

    for(uint32_t i = 0; i < widgets.size(); ++i) {
        Widget &w = widgets[i];
        w.position = new_positions[i];
    }

    // ranges

    for(uint32_t i = 0; i < widgets.size(); ++i) {
        vec4 range = vec4(-FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX);

        uint32_t current = i;
        while(true) {
            Widget& w = widgets[current];

            if(current != i) {
                vec4 wrange = vec4(w.position, w.position + w.size);
                range = clip(range, wrange);
            }

            if(w.parent == NULL_ENTITY || w.position_mode == PM_STATIC) break;
            else current = w.parent;
        }

        widgets[i].range = range;
    }


    //

    for(Widget& w : widgets) {
        w.mesh_func(w);
        w.input_func(w);

        vertices.insert(vertices.end(), w.vertices.begin(), w.vertices.end());
    }

    //

    std::vector<std::string> ks;
    for(auto& [k, state] : window_state) {
        if(!state.active) ks.push_back(k);
    }

    for(auto k : ks) window_state.erase(k);
}

std::vector<UI_vertex> create_char(Glyph_data& glyph) {
    std::vector<UI_vertex> ret;

    UI_vertex a = {vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec4(1.0f)};
    UI_vertex b = {vec2(1.0f, 0.0f), vec2(1.0f, 0.0f), vec4(1.0f)};
    UI_vertex c = {vec2(0.0f, 1.0f), vec2(0.0f, 1.0f), vec4(1.0f)};
    UI_vertex d = {vec2(1.0f, 1.0f), vec2(1.0f, 1.0f), vec4(1.0f)};

    ret.push_back(a);
    ret.push_back(b);
    ret.push_back(d);
    ret.push_back(a);
    ret.push_back(d);
    ret.push_back(c);

    for(UI_vertex& v : ret) {
        v.pos = v.pos * vec2(glyph.size[0], glyph.size[1]);
        v.tex_pos = vec2(glyph.pos_tex[0], glyph.pos_tex[1]) + v.tex_pos * vec2(glyph.size[0], glyph.size[1]);
    }

    return ret;
}

float get_text_y(Font& f, std::string text, uint32_t text_size, uint32_t width, TEXT_ALIGN alignment) {
    std::vector<UI_vertex> ret;

    bool show_debug = false;

    GUI_system& gui_system = ecs.get_system<GUI_system>();

    uint32_t line_start_index = 0;
    uint32_t word_start_index = 0;

    bool accept_index = false;

    int num_escape_seq = 0;
    int i = 0;

    vec2 pos = vec2(0.0f);
    vec4 range = vec4(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);

    vec4 color = vec4(1.0f);
    bool bold = false;
    bool italic = false;
    bool hex = false;

    uint32_t num_lines = 0;

    vec2 word_pos = vec2(0.0f);

    auto insert_line = [&]() {
        int line_width = pos.x;
        int offset;

        pos.x = 0;
        pos.y -= float(f.line_height) * text_size;
        ++num_lines;
    };

    auto insert_word = [&]() {
        uint32_t end = pos.x + word_pos.x;

        if(end > width) {
            insert_line();
        }

        pos.x += word_pos.x;
        word_pos = vec2(0.0f);
    };

    auto insert_char = [&](char c) {
        Glyph_data& gd = f.at(c);

        float stride = gd.stride;

        if(!gd.visible) {
            if(alignment == TE_LEFT) {
                word_pos.x += stride * text_size;

                insert_word();
            } else if(alignment == TE_CENTER) {
                insert_word();

                word_pos.x += stride * text_size;

                insert_word();
            } else if(alignment == TE_RIGHT) {
                insert_word();

                word_pos.x += stride * text_size;
            }
        } else {
            if(bold) {
                stride += bold_factor;
            }

            word_pos.x += stride * text_size;
        }
    };

    for(i = 0; i < text.size(); ++i) {
        char c = text[i];

        if(c == '\n') {
            insert_word();
            insert_line();
            word_start_index = i;
            line_start_index = i;

            continue;
        } else {
            if(c == '\\') {
                if(i + 1 < text.size()) {
                    char next = text[i + 1];

                    if(next == 'c') {
                        if(i + 1 + 3 < text.size()) {
                            std::string s(text.begin() + (i + 2), text.begin() + (i + 5));

                            std::size_t i0 = integers_letters.find(s[0]);
                            std::size_t i1 = integers_letters.find(s[1]);
                            std::size_t i2 = integers_letters.find(s[2]);

                            if(i0 != std::string::npos && i1 != std::string::npos && i2 != std::string::npos) {
                                color = vec4(float(i0) / 15.0f, float(i1) / 15.0f, float(i2) / 15.0f, 1.0f);

                                num_escape_seq += 5;
                                if(!show_debug) {
                                    i += 4;
                                    continue;
                                }
                            }
                        }
                    } else if(next == 'b') {
                        bold = true;

                        num_escape_seq += 2;
                        if(!show_debug) {
                            i += 1;
                            continue;
                        }
                    } else if(next == 'i') {
                        italic = true;

                        num_escape_seq += 2;
                        if(!show_debug) {
                            i += 1;
                            continue;
                        }

                    } else if(next == 'r') {
                        bold = false;
                        italic = false;

                        num_escape_seq += 2;
                        if(!show_debug) {
                            i += 1;
                            continue;
                        }
                    } else if(next == 'h') {
                        hex = !hex;

                        num_escape_seq += 2;
                        if(!show_debug) {
                            i += 1;
                            continue;
                        }
                    }
                }
            }

            if(show_debug) {
                if(num_escape_seq > 0) {
                    color.w = 0.5f;
                    --num_escape_seq;

                    if(c == 'A') c = '\x80';
                    else if(c == 'B') c = '\x81';
                    else if(c == 'C') c = '\x82';
                    else if(c == 'D') c = '\x83';
                    else if(c == 'E') c = '\x84';
                    else if(c == 'F') c = '\x85';
                } else color.w = 1.0f;
            }

            if(hex) {
                if(c == 'A') c = '\x80';
                else if(c == 'B') c = '\x81';
                else if(c == 'C') c = '\x82';
                else if(c == 'D') c = '\x83';
                else if(c == 'E') c = '\x84';
                else if(c == 'F') c = '\x85';
            }

            insert_char(c);
        }
    }

    insert_word();
    insert_line();

    return num_lines * f.line_height * text_size;
}

std::vector<UI_vertex> mesh_text(Font& f, std::string text, uint32_t text_size, uint32_t width, TEXT_ALIGN alignment) {
    std::vector<UI_vertex> ret;

    bool show_debug = false;

    GUI_system& gui_system = ecs.get_system<GUI_system>();

    uint32_t line_start_index = 0;
    uint32_t word_start_index = 0;

    bool accept_index = false;

    int num_escape_seq = 0;
    int i = 0;

    vec2 pos = vec2(0.0f);
    vec4 range = vec4(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);

    vec4 color = vec4(1.0f);
    bool bold = false;
    bool italic = false;
    bool hex = false;

    uint32_t num_lines = 0;

    std::vector<UI_vertex> word_ret;
    vec2 word_pos = vec2(0.0f);

    std::vector<UI_vertex> line_ret;

    auto insert_line = [&]() {
        int line_width = pos.x;
        int offset;

        if(alignment == TE_LEFT) offset = 0.0f;
        else if(alignment == TE_CENTER) offset = round(float(int(width) - line_width) / 2);
        else if(alignment == TE_RIGHT) offset = int(width) - line_width;

        for(UI_vertex& v : line_ret) {
            v.pos.x += offset;
        }

        ret.insert(ret.end(), line_ret.begin(), line_ret.end());

        line_ret.clear();

        pos.x = 0;
        pos.y -= f.line_height * text_size;
        ++num_lines;
    };

    auto insert_word = [&]() {
        uint32_t end = pos.x + word_pos.x;

        if(end > width) {
            insert_line();
        }

        if(line_ret.size() == 0) line_start_index = word_start_index;

        // insert word
        for(UI_vertex& v : word_ret) {
            v.pos += pos;
        }

        line_ret.insert(line_ret.end(), word_ret.begin(), word_ret.end());

        word_ret.clear();

        pos.x += word_pos.x;
        word_pos = vec2(0.0f);
    };;

    auto insert_char = [&](char c) {
        Glyph_data& gd = f.at(c);

        float stride = gd.stride;

        if(word_ret.size() == 0) {
            word_start_index = i;
        }

        if(!gd.visible) {
            UI_vertex v;
            v.pos = word_pos + vec2(stride, 0);
            v.data = 0xFFFFFFFF;
            word_ret.push_back(v);
            word_ret.push_back(v);
            word_ret.push_back(v);

            if(alignment == TE_LEFT) {
                word_pos.x += stride * text_size;

                insert_word();
            } else if(alignment == TE_CENTER) {
                insert_word();

                word_pos.x += stride * text_size;

                insert_word();
            } else if(alignment == TE_RIGHT) {
                insert_word();

                word_pos.x += stride * text_size;
            }
        } else {
            std::vector<UI_vertex> vs = create_char(gd);

            for(UI_vertex& v : vs) v.pos *= float(text_size);

            for(UI_vertex& v : vs) {
                v.pos += word_pos;
            }

            for(UI_vertex& v : vs) {
                if(italic) {
                    v.pos.x += float(v.pos.y - word_pos.y - f.line_height * 0.5f) * italic_factor * text_size;
                }

                v.color = color;
            }

            word_ret.insert(word_ret.end(), vs.begin(), vs.end());

            if(bold) {
                for(UI_vertex& v : vs) {
                    v.pos.x += bold_factor * text_size;
                }
                stride += bold_factor * text_size;

                word_ret.insert(word_ret.end(), vs.begin(), vs.end());
            }

            word_pos.x += stride * text_size;
        }
    };

    for(i = 0; i < text.size(); ++i) {
        char c = text[i];

        if(c == '\n') {
            insert_word();
            insert_line();
            word_start_index = i;
            line_start_index = i;

            continue;
        } else {
            if(c == '\\') {
                if(i + 1 < text.size()) {
                    char next = text[i + 1];

                    if(next == 'c') {
                        if(i + 1 + 3 < text.size()) {
                            std::string s(text.begin() + (i + 2), text.begin() + (i + 5));

                            std::size_t i0 = integers_letters.find(s[0]);
                            std::size_t i1 = integers_letters.find(s[1]);
                            std::size_t i2 = integers_letters.find(s[2]);

                            if(i0 != std::string::npos && i1 != std::string::npos && i2 != std::string::npos) {
                                color = vec4(float(i0) / 15.0f, float(i1) / 15.0f, float(i2) / 15.0f, 1.0f);

                                num_escape_seq += 5;
                                if(!show_debug) {
                                    i += 4;
                                    continue;
                                }
                            }
                        }
                    } else if(next == 'b') {
                        bold = true;

                        num_escape_seq += 2;
                        if(!show_debug) {
                            i += 1;
                            continue;
                        }
                    } else if(next == 'i') {
                        italic = true;

                        num_escape_seq += 2;
                        if(!show_debug) {
                            i += 1;
                            continue;
                        }

                    } else if(next == 'r') {
                        bold = false;
                        italic = false;

                        num_escape_seq += 2;
                        if(!show_debug) {
                            i += 1;
                            continue;
                        }
                    } else if(next == 'h') {
                        hex = !hex;

                        num_escape_seq += 2;
                        if(!show_debug) {
                            i += 1;
                            continue;
                        }
                    }
                }
            }

            if(show_debug) {
                if(num_escape_seq > 0) {
                    color.w = 0.5f;
                    --num_escape_seq;

                    if(c == 'A') c = '\x80';
                    else if(c == 'B') c = '\x81';
                    else if(c == 'C') c = '\x82';
                    else if(c == 'D') c = '\x83';
                    else if(c == 'E') c = '\x84';
                    else if(c == 'F') c = '\x85';
                } else color.w = 1.0f;
            }

            if(hex) {
                if(c == 'A') c = '\x80';
                else if(c == 'B') c = '\x81';
                else if(c == 'C') c = '\x82';
                else if(c == 'D') c = '\x83';
                else if(c == 'E') c = '\x84';
                else if(c == 'F') c = '\x85';
            }

            insert_char(c);
        }
    }

    insert_word();
    insert_line();

    for(UI_vertex& v : ret) {
        range.x = min(range.x, v.pos.x);
        range.y = min(range.y, v.pos.y);
        range.z = max(range.z, v.pos.x);
        range.w = max(range.w, v.pos.y);

        v.pos.y += (num_lines - 1) * f.line_height * text_size;
    }

    return ret;
}

bool contains(vec2 pos, ivec4 range) {
    return pos.x >= range.x && pos.x < range.z && pos.y >= range.y && pos.y < range.w;
}

void GUI_system::window(std::string name, Window_state state, std::function<void()> on_close) {
    if(window_state.find(name) == window_state.end()) {
        window_state[name] = state;
    }

    //

    Window_state& ws = window_state[name];
    ws.active = true;
    float header = 48.0f;
    vec2& position = ws.position;
    vec2& size = ws.size;

    //

    Widget widget;
    widget.size = ws.size;
    widget.position = ws.position;

    //

    widget.input_func = [this, header, name, on_close, &ws](Widget& widget) {
        float content_height = widget.data.back();
        float window_height = widget.size.y;

        //

        Input_system& input_system = ecs.get_system<Input_system>();

        int buffer = 16.0f;
        ivec4 buffer_range = ivec4(-buffer, -buffer, buffer, buffer);

        ivec4 range_move = {widget.position + vec2(0, widget.size.y), widget.position + vec2(widget.size.x, widget.size.y + header)};
        range_move += buffer_range;

        buffer = 24.0f;
        buffer_range = ivec4(-buffer, -buffer, buffer, buffer);

        ivec4 range_left = {widget.position + vec2(0, 0), widget.position + vec2(0, widget.size.y)};
        ivec4 range_right = {widget.position + vec2(widget.size.x, 0), widget.position + vec2(widget.size.x, widget.size.y)};
        ivec4 range_bottom = {widget.position + vec2(0, 0), widget.position + vec2(widget.size.x, 0)};

        ivec4 range_close = {widget.position + vec2(widget.size.x - header, widget.size.y), widget.position + vec2(widget.size.x, widget.size.y + header)};

        range_left += buffer_range;
        range_right += buffer_range;
        range_bottom += buffer_range;

        vec2 min_size = vec2(256, 128);

        if(capture_id != 0xFFFFFFFF && capture_widget == name) {
            Pointer& pointer = input_system.pointers.at(capture_id);

            float delta_min;

            switch(capture_operation) {
                case 0:
                    ws.position += pointer.pos - pointer.prev_pos;
                    break;
                case 1:
                    ws.position.x += pointer.pos.x - pointer.prev_pos.x;
                    ws.size.x -= pointer.pos.x - pointer.prev_pos.x;

                    if(pointer.pos.x + buffer > ws.position.x) {
                        float delta_max = (pointer.pos.x + buffer) - ws.position.x;
                        ws.position.x += delta_max;
                        ws.size.x -= delta_max;
                    }

                    delta_min = max(0.0f, min_size.x - ws.size.x);
                    ws.size.x += delta_min;
                    ws.position.x -= delta_min;

                    break;
                case 2:
                    ws.size.x += pointer.pos.x - pointer.prev_pos.x;

                    if(pointer.pos.x - buffer < ws.position.x + ws.size.x) {
                        float delta_max = (ws.position.x + ws.size.x) - (pointer.pos.x - buffer);
                        ws.size.x -= delta_max;
                    }

                    delta_min = max(0.0f, min_size.x - ws.size.x);
                    ws.size.x += delta_min;

                    break;
                case 3:
                    ws.position.y += pointer.pos.y - pointer.prev_pos.y;
                    ws.size.y -= pointer.pos.y - pointer.prev_pos.y;

                    if(pointer.pos.y + buffer > ws.position.y) {
                        float delta_max = (pointer.pos.y + buffer) - ws.position.y;
                        ws.position.y += delta_max;
                        ws.size.y -= delta_max;
                    }

                    delta_min = max(0.0f, min_size.y - ws.size.y);
                    ws.size.y += delta_min;
                    ws.position.y -= delta_min;

                    break;
                case 4:
                    // x axis
                    ws.position.x += pointer.pos.x - pointer.prev_pos.x;
                    ws.size.x -= pointer.pos.x - pointer.prev_pos.x;

                    if(pointer.pos.x + buffer > ws.position.x) {
                        float delta_max = (pointer.pos.x + buffer) - ws.position.x;
                        ws.position.x += delta_max;
                        ws.size.x -= delta_max;
                    }

                    delta_min = max(0.0f, min_size.x - ws.size.x);
                    ws.size.x += delta_min;
                    ws.position.x -= delta_min;

                    // y axis
                    ws.position.y += pointer.pos.y - pointer.prev_pos.y;
                    ws.size.y -= pointer.pos.y - pointer.prev_pos.y;

                    if(pointer.pos.y + buffer > ws.position.y) {
                        float delta_max = (pointer.pos.y + buffer) - ws.position.y;
                        ws.position.y += delta_max;
                        ws.size.y -= delta_max;
                    }

                    delta_min = max(0.0f, min_size.y - ws.size.y);
                    ws.size.y += delta_min;
                    ws.position.y -= delta_min;

                    break;
                case 5:
                    // x axis
                    ws.size.x += pointer.pos.x - pointer.prev_pos.x;

                    if(pointer.pos.x - buffer < ws.position.x + ws.size.x) {
                        float delta_max = (ws.position.x + ws.size.x) - (pointer.pos.x - buffer);
                        ws.size.x -= delta_max;
                    }

                    delta_min = max(0.0f, min_size.x - ws.size.x);
                    ws.size.x += delta_min;

                    // y axis
                    ws.position.y += pointer.pos.y - pointer.prev_pos.y;
                    ws.size.y -= pointer.pos.y - pointer.prev_pos.y;

                    if(pointer.pos.y + buffer > ws.position.y) {
                        float delta_max = (pointer.pos.y + buffer) - ws.position.y;
                        ws.position.y += delta_max;
                        ws.size.y -= delta_max;
                    }

                    delta_min = max(0.0f, min_size.y - ws.size.y);
                    ws.size.y += delta_min;
                    ws.position.y -= delta_min;

                    break;
            }
        }

        // scrollbar

        if(content_height <= window_height) {
            ws.scroll = 0;
            widget.child_offset.y = 0;
        } else {
            float ratio = window_height / content_height;
            float open_space = 1.0f - ratio;

            float bar_height = ratio * window_height;
            float scroll_range = open_space * window_height;

            float bar_position = (1.0f - ws.scroll / (content_height - window_height)) * scroll_range;

            vec4 bar_range = vec4(0.0f, bar_position, 18.0f, bar_position + bar_height);

            vec2 offset = widget.position + vec2(widget.size.x, 0.0f);
            bar_range += vec4(offset, offset);
        }

        if(capture_id == 0xFFFFFFFF) {
            for(auto &[key, pointer]: input_system.pointers) {
                if(pointer.down < 2) {
                    if(contains(pointer.pos, range_move)) {
                        capture_id = key;
                        capture_widget = name;
                        capture_operation = 0;
                    }

                    if(contains(pointer.pos, range_close)) {
                        on_close();
                    }

                    bool left_cont = contains(pointer.pos, range_left);
                    bool right_cont = contains(pointer.pos, range_right);
                    bool bottom_cont = contains(pointer.pos, range_bottom);

                    if(left_cont && bottom_cont) {
                        capture_id = key;
                        capture_widget = name;
                        capture_operation = 4;
                    } else if(right_cont && bottom_cont) {
                        capture_id = key;
                        capture_widget = name;
                        capture_operation = 5;
                    } else if(left_cont) {
                        capture_id = key;
                        capture_widget = name;
                        capture_operation = 1;
                    } else if(right_cont) {
                        capture_id = key;
                        capture_widget = name;
                        capture_operation = 2;
                    } else if(bottom_cont) {
                        capture_id = key;
                        capture_widget = name;
                        capture_operation = 3;
                    }
                }
            }
        }
    };

    widget.mesh_func = [this, header, ws](Widget& widget) {
        float scrollbar_width = 18.0f;

        vec4 header_range = vec4(widget.position + vec2(0.0f, widget.size.y), widget.position + vec2(widget.size.x, widget.size.y + header));

        std::vector<UI_vertex> total_ret;

        UI_vertex a = {vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec4(1.0f)};
        UI_vertex b = {vec2(1.0f, 0.0f), vec2(1.0f, 0.0f), vec4(1.0f)};
        UI_vertex c = {vec2(0.0f, 1.0f), vec2(0.0f, 1.0f), vec4(1.0f)};
        UI_vertex d = {vec2(1.0f, 1.0f), vec2(1.0f, 1.0f), vec4(1.0f)};


        // main window
        std::vector<UI_vertex> ret = {a, b, d, a, d, c};
        for(UI_vertex& v : ret) {
            v.pos = widget.position + vec2(0.0, 0.0f) + v.pos * widget.size;
            v.tex_pos = vec2(1.0f);
            v.color = vec4(0.35f, 0.35f, 0.35f, 0.35f);
            v.data = 1;
        }
        total_ret.insert(total_ret.end(), ret.begin(), ret.end());

        // header
        ret = {a, b, d, a, d, c};
        for(UI_vertex& v : ret) {
            v.pos = widget.position + vec2(0.0, widget.size.y) + v.pos * vec2(widget.size.x + scrollbar_width, header);
            v.tex_pos = vec2(1.0f);
            v.color = vec4(1.0f, 0.35f, 0.35f, 1.0f);
            v.data = 1;
        }
        total_ret.insert(total_ret.end(), ret.begin(), ret.end());

        // scrollbar
        ret = {a, b, d, a, d, c};
        for(UI_vertex& v : ret) {
            v.pos = widget.position + vec2(widget.size.x, 0.0) + v.pos * vec2(scrollbar_width, widget.size.y);
            v.tex_pos = vec2(1.0f);
            v.color = vec4(0.25f, 0.25, 0.25f, 1.0f);
            v.data = 1;
        }
        total_ret.insert(total_ret.end(), ret.begin(), ret.end());

        // scrollbar widget
        float content_height = widget.data.back();
        float window_height = widget.size.y;
        if(content_height >= window_height) {
            float ratio = window_height / content_height;
            float open_space = 1.0f - ratio;

            float bar_height = ratio * window_height;
            float scroll_range = open_space * window_height;

            float bar_position = (1.0f - ws.scroll / (content_height - window_height)) * scroll_range;

            vec4 bar_range = vec4(0.0f, bar_position, 18.0f, bar_position + bar_height);

            vec2 offset = widget.position + vec2(widget.size.x, 0.0f);
            bar_range += vec4(offset, offset);

            ret = {a, b, d, a, d, c};
            for(UI_vertex& v : ret) {
                v.pos = vec2(bar_range.x, bar_range.y) + v.pos * vec2(bar_range.z - bar_range.x, bar_range.w - bar_range.y);
                v.tex_pos = vec2(1.0f);
                v.color = vec4(0.75f, 0.75, 0.75f, 1.0f);
                v.data = 1;
            }
            total_ret.insert(total_ret.end(), ret.begin(), ret.end());
        }

        // text
        ret = mesh_text(fonts["default_mono"], ws.label, 3, 0xFFFFFFFF, TE_LEFT);

        uint32_t tex_scale = 3.0f;

        for(UI_vertex& v : ret) {
            float s = floor(header * 0.5f - 11.0f * float(tex_scale) * 0.5f);
            v.pos = widget.position + vec2(0.0f, widget.size.y + header) + v.pos + vec2(s, -s - 11.0f * float(tex_scale));
            v.range = header_range;
        }
        total_ret.insert(total_ret.end(), ret.begin(), ret.end());

        vec4 range = vec4(0, 2, 5, 7);

        // x
        ret = {a, b, d, a, d, c};
        for(UI_vertex& v : ret) {
            float s = floor(header * 0.5f - 5.0f * float(tex_scale) * 0.5f);
            vec2 icon_size = vec2(range.z - range.x, range.w - range.y);

            v.pos = widget.position + vec2(0.0f, widget.size.y + header) + v.pos * float(tex_scale) * icon_size + vec2(widget.size.x + scrollbar_width - s - 5.0f * float(tex_scale), -s - 5.0f * float(tex_scale));
            v.tex_pos = v.tex_pos * icon_size + vec2(range.x, range.y);
            v.color = vec4(1.0f);
            v.data = 1;
            v.range = header_range;
        }
        total_ret.insert(total_ret.end(), ret.begin(), ret.end());

        vertices.insert(vertices.end(), total_ret.begin(), total_ret.end());

        //

        widget.vertices = total_ret;
    };

    widget_path.resize(1);
    widget.parent = 0;
    widgets[widget_path.back()].children.push_back(widgets.size());
    widget_path.push_back(widgets.size());

    widgets.push_back(widget);
}

void GUI_system::button(vec2 size, ivec4 icon, vec4 color, std::function<void()> on_click) {
    Widget widget;
    widget.size = size;

    widget.buffer = widget_buffer;
    widget.position_mode = position_mode;

    widget.input_func = [this, on_click](Widget& widget) {
        Input_system &input_system = ecs.get_system<Input_system>();

        ivec4 range = ivec4(widget.position, widget.position + widget.size);
        if (capture_id == 0xFFFFFFFF) {
            for (auto &[key, pointer]: input_system.pointers) {
                if (pointer.down < 2) {
                    if (contains(pointer.pos, range)) {
                        capture_id = key;
                        capture_operation = 10;

                        on_click();
                    }
                }
            }
        }
    };

    widget.mesh_func = [this, color, icon](Widget& widget) {
        std::vector<UI_vertex> total_ret;

        UI_vertex a = {vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec4(1.0f)};
        UI_vertex b = {vec2(1.0f, 0.0f), vec2(1.0f, 0.0f), vec4(1.0f)};
        UI_vertex c = {vec2(0.0f, 1.0f), vec2(0.0f, 1.0f), vec4(1.0f)};
        UI_vertex d = {vec2(1.0f, 1.0f), vec2(1.0f, 1.0f), vec4(1.0f)};

        std::vector<UI_vertex> ret = {a, b, d, a, d, c};
        for (UI_vertex &v: ret) {
            v.pos = widget.position + v.pos * widget.size;
            v.tex_pos = vec2(1.0f);
            v.color = color;
            v.data = 1;
            v.range = widget.range;
        }
        total_ret.insert(total_ret.end(), ret.begin(), ret.end());

        vec2 size_tex = vec2(icon.z - icon.x, icon.w - icon.y);
        float scale_tex = 6.0f;

        float buffer = floor((widget.size.y - size_tex.y * scale_tex) * 0.5f);

        ret = {a, b, d, a, d, c};
        for (UI_vertex &v: ret) {
            v.pos = widget.position + vec2(widget.size.x - buffer - size_tex.x * scale_tex, buffer) +
                    v.pos * size_tex * scale_tex;
            v.tex_pos = v.tex_pos * size_tex + vec2(icon.x, icon.y);
            v.color = vec4(1.0f);
            v.data = 1;
            v.range = widget.range;
        }
        total_ret.insert(total_ret.end(), ret.begin(), ret.end());

        widget.vertices = total_ret;
    };

    widget.parent = widget_path.back();
    widgets[widget_path.back()].children.push_back(widgets.size());

    widgets.push_back(widget);
}

void GUI_system::text(std::string text) {
    Widget widget;
    widget.size = vec2(0.0f);
    widget.position = vec2(0.0f);

    widget.position_mode = PM_VOID;
    widget.size_mode = SM_FILL;

    // get min size

    auto& font = fonts["default_mono"];
    float w = 0;

    for(char c : text) {
        Glyph_data& gd = font.at(c);

        w += float(gd.stride);
    }

    widget.size.x = w;

    //

    widget.input_func = [](Widget& widget) {
    };

    widget.mesh_func = [this, text](Widget& widget) {
        uint32_t tex_scale = 3.0f;

        std::vector<UI_vertex> ret = mesh_text(fonts["default_mono"], text, 3, 0xFFFFFFFF, TE_LEFT);

        for(UI_vertex& v : ret) {
            //v.pos = current_pos + v.pos * float(tex_scale) + vec2(0.0f, -11.0f * float(tex_scale));
            v.range = current_range;
        }

        widget.vertices = ret;
    };

    widget.parent = widget_path.back();
    widgets[widget_path.back()].children.push_back(widgets.size());
    widgets.push_back(widget);
}

void GUI_system::row() {
    Widget widget;
    widget.size = vec2(0.0f);
    widget.position = vec2(0.0f);

    widget.buffer = widget_buffer;
    widget.position_mode = position_mode;

    widget.layout_mode = LM_ROW;
    widget.size_mode = SM_SURROUND;

    widget.parent = widget_path.back();
    widgets[widget_path.back()].children.push_back(widgets.size());
    widget_path.push_back(widgets.size());

    widgets.push_back(widget);
}

void GUI_system::column() {
    Widget widget;
    widget.size = vec2(0.0f);
    widget.position = vec2(0.0f);

    widget.buffer = widget_buffer;
    widget.position_mode = position_mode;

    widget.layout_mode = LM_COLUMN;
    widget.size_mode = SM_SURROUND;

    widget.parent = widget_path.back();
    widgets[widget_path.back()].children.push_back(widgets.size());
    widget_path.push_back(widgets.size());

    widgets.push_back(widget);
}

void GUI_system::grid(ivec2 size) {
    Widget widget;
    widget.size = vec2(0.0f);
    widget.position = vec2(0.0f);

    widget.buffer = widget_buffer;
    widget.position_mode = position_mode;

    widget.layout_mode = LM_GRID;
    widget.size_mode = SM_SURROUND;

    widget.data = {size.x, size.y};

    widget.parent = widget_path.back();
    widgets[widget_path.back()].children.push_back(widgets.size());
    widget_path.push_back(widgets.size());

    widgets.push_back(widget);
}

void GUI_system::debug_square(vec2 size, vec4 color, bool fill) {
    Widget widget;
    widget.size = size;

    widget.buffer = widget_buffer;
    widget.position_mode = position_mode;
    if(fill) widget.size_mode = SM_FILL;

    widget.mesh_func = [this, color](Widget& widget) {
        std::vector<UI_vertex> total_ret;

        UI_vertex a = {vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec4(1.0f)};
        UI_vertex b = {vec2(1.0f, 0.0f), vec2(1.0f, 0.0f), vec4(1.0f)};
        UI_vertex c = {vec2(0.0f, 1.0f), vec2(0.0f, 1.0f), vec4(1.0f)};
        UI_vertex d = {vec2(1.0f, 1.0f), vec2(1.0f, 1.0f), vec4(1.0f)};

        std::vector<UI_vertex> ret = {a, b, d, a, d, c};
        for (UI_vertex &v: ret) {
            v.pos = widget.position + v.pos * widget.size;
            v.tex_pos = vec2(1.0f);
            v.color = color;
            v.data = 1;
            v.range = widget.range;
        }
        total_ret.insert(total_ret.end(), ret.begin(), ret.end());

        widget.vertices = total_ret;
    };

    widget.parent = widget_path.back();
    widgets[widget_path.back()].children.push_back(widgets.size());

    if(fill) {
        widget.size_func = [](Widget &w) {
            GUI_system& gui_system = ecs.get_system<GUI_system>();

            std::string string = R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit. Duis odio nibh, vestibulum a posuere sed, egestas ac augue. Praesent quis commodo augue. Duis iaculis suscipit quam, at scelerisque turpis egestas non. Suspendisse venenatis lectus ut malesuada tempus. Suspendisse tincidunt, justo et maximus ultricies, erat sapien ultricies mi, non fermentum risus nisl nec neque. Suspendisse auctor tortor non mattis consectetur. Morbi a augue hendrerit, consequat erat molestie, elementum turpis. Vestibulum convallis eleifend urna. Aenean auctor, ex a dictum sodales, ipsum arcu blandit neque, vitae sodales sapien lorem volutpat lacus. Aliquam rhoncus nisl elit, id tincidunt mi mattis sed. Suspendisse in ornare eros. Duis vehicula ex enim, ut gravida nisi tempus in. Nulla finibus, enim nec placerat facilisis, ante nunc commodo nisl, eget rutrum dui ipsum in nunc. Proin molestie elit quis nisl sagittis tristique.

                Aliquam ipsum lacus, commodo vitae quam rhoncus, auctor suscipit urna. Aenean laoreet neque maximus pharetra dictum. Nulla vestibulum purus vitae tortor semper sagittis. Nulla ultricies, magna lacinia fringilla accumsan, dolor mi venenatis leo, ac imperdiet ex lacus et neque. Ut id luctus turpis. Donec lobortis rhoncus lorem, eu feugiat leo mollis vel. Nulla accumsan volutpat leo et iaculis. Pellentesque a iaculis orci. Sed eu rhoncus neque. Fusce sit amet congue turpis, id dapibus purus. Curabitur nulla orci, tincidunt vel quam eu, molestie rhoncus lacus. Nulla a magna id turpis consectetur consequat. Duis sit amet nisi dictum, tempor mi at, gravida dolor. Praesent accumsan elit augue, quis ultricies ex feugiat lobortis.

                Nulla id pulvinar ligula, id tempus nisl. Duis et ante pharetra, blandit ligula sed, ullamcorper odio. Donec ultrices ligula non ultricies lacinia. Suspendisse potenti. Suspendisse leo nibh, euismod sed dui at, finibus tempus ligula. Donec euismod magna ac nulla dignissim congue. Sed lobortis ac eros in bibendum. Duis vestibulum ante a ligula lacinia tempor. Pellentesque sodales nunc.)";
;

            w.size.y = get_text_y(gui_system.fonts["default_mono"], string, 2, w.size.x, TE_LEFT);
        };

        widget.mesh_func = [this, color](Widget& widget) {
            GUI_system& gui_system = ecs.get_system<GUI_system>();
            std::string string = R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit. Duis odio nibh, vestibulum a posuere sed, egestas ac augue. Praesent quis commodo augue. Duis iaculis suscipit quam, at scelerisque turpis egestas non. Suspendisse venenatis lectus ut malesuada tempus. Suspendisse tincidunt, justo et maximus ultricies, erat sapien ultricies mi, non fermentum risus nisl nec neque. Suspendisse auctor tortor non mattis consectetur. Morbi a augue hendrerit, consequat erat molestie, elementum turpis. Vestibulum convallis eleifend urna. Aenean auctor, ex a dictum sodales, ipsum arcu blandit neque, vitae sodales sapien lorem volutpat lacus. Aliquam rhoncus nisl elit, id tincidunt mi mattis sed. Suspendisse in ornare eros. Duis vehicula ex enim, ut gravida nisi tempus in. Nulla finibus, enim nec placerat facilisis, ante nunc commodo nisl, eget rutrum dui ipsum in nunc. Proin molestie elit quis nisl sagittis tristique.

                Aliquam ipsum lacus, commodo vitae quam rhoncus, auctor suscipit urna. Aenean laoreet neque maximus pharetra dictum. Nulla vestibulum purus vitae tortor semper sagittis. Nulla ultricies, magna lacinia fringilla accumsan, dolor mi venenatis leo, ac imperdiet ex lacus et neque. Ut id luctus turpis. Donec lobortis rhoncus lorem, eu feugiat leo mollis vel. Nulla accumsan volutpat leo et iaculis. Pellentesque a iaculis orci. Sed eu rhoncus neque. Fusce sit amet congue turpis, id dapibus purus. Curabitur nulla orci, tincidunt vel quam eu, molestie rhoncus lacus. Nulla a magna id turpis consectetur consequat. Duis sit amet nisi dictum, tempor mi at, gravida dolor. Praesent accumsan elit augue, quis ultricies ex feugiat lobortis.

                Nulla id pulvinar ligula, id tempus nisl. Duis et ante pharetra, blandit ligula sed, ullamcorper odio. Donec ultrices ligula non ultricies lacinia. Suspendisse potenti. Suspendisse leo nibh, euismod sed dui at, finibus tempus ligula. Donec euismod magna ac nulla dignissim congue. Sed lobortis ac eros in bibendum. Duis vestibulum ante a ligula lacinia tempor. Pellentesque sodales nunc.)";

            std::vector<UI_vertex> vs = mesh_text(gui_system.fonts["default_mono"], string, 2, widget.size.x, TE_LEFT);
            for(UI_vertex& v : vs) {
                v.pos += widget.position;
            }

            widget.vertices = vs;
        };
    }

    widgets.push_back(widget);
}

void GUI_system::set_mode(POSITION_MODE mode) {
    position_mode = mode;
}

void GUI_system::set_buffer(vec2 buffer) {
    widget_buffer = buffer;
}

void GUI_system::reset() {
    widget_path = {0};
}

void GUI_system::step() {
    widget_path.resize(widget_path.size() - 1);
}


/*
Widget widget;
widget.size = ;
widget.position = ;

widget.input_func = [](Widget& widget) {
};

widget.mesh_func = [](Widget& widget) {
};

widgets.push_back(widget);
*/
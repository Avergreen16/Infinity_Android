//
// Created by plane on 2/14/2026.
//

#include "gui.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"

std::string numbers = "0123456789\x80\x81\x82\x83\x84\x85";

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
    fonts.emplace("default_mono", Font("fonts/alter_mono.afont"));
}

void GUI_system::call() {
    vertices.clear();

    Renderer& renderer = ecs.get_system<Renderer>();
    Input_system& input_system = ecs.get_system<Input_system>();

    if(input_system.pointers.find(capture_id) == input_system.pointers.end()) {
        capture_id = 0xFFFFFFFF;
    }

    Transform2D& ct = ecs.get_component<Transform2D>(renderer.camera);

    int width, height;
    eglQuerySurface(renderer.display, renderer.surface, EGL_WIDTH, &width);
    eglQuerySurface(renderer.display, renderer.surface, EGL_HEIGHT, &height);

    static bool open_settings;
    static bool open_chat;
    static bool open_stats;

    if(!init_gui) {
        init_gui = true;

        open_settings = false;
        open_chat = false;
        open_stats = false;
    }

    if(open_settings) {
        tab({0.0f, height - 64.0f - 96.0f}, {160.0f, 96.0f}, {54, 54, 64, 64}, open_settings);
    } else {
        tab({0.0f, height - 64.0f - 96.0f}, {96.0f, 96.0f}, {54, 54, 64, 64}, open_settings);
        if(open_settings) {
            //open_chat = false;
            //window_state.clear();

            Window_state ws;
            ws.size = {512.0f, 768.0f};
            ws.position = {width * 0.5f - ws.size.x * 0.5f, height * 0.5f + ws.size.y * 0.5f};
            ws.label = "settings";

            window_state["settings_window"] = ws;
        }
    }

    if(open_stats) {
        tab({0.0f, height - 64.0f - 96.0f * 2.0f - 32.0f}, {160.0f, 96.0f}, {54, 34, 64, 44},open_stats);
    } else {
        tab({0.0f, height - 64.0f - 96.0f * 2.0f - 32.0f}, {96.0f, 96.0f}, {54, 34, 64, 44},open_stats);
        if(open_stats) {
            //open_settings = false;
            //window_state.clear();

            Window_state ws;
            ws.size = {512.0f, 768.0f};
            ws.position = {width * 0.5f - ws.size.x * 0.5f, height * 0.5f + ws.size.y * 0.5f};
            ws.label = "statistics";

            window_state["stats_window"] = ws;
        }
    }

    if(open_chat) {
        tab({0.0f, height - 64.0f - 96.0f * 3.0f - 32.0f * 2.0f}, {160.0f, 96.0f}, {54, 44, 64, 54},open_chat);
    } else {
        tab({0.0f, height - 64.0f - 96.0f * 3.0f - 32.0f * 2.0f}, {96.0f, 96.0f}, {54, 44, 64, 54},open_chat);
        if(open_chat) {
            //open_settings = false;
            //window_state.clear();

            Window_state ws;
            ws.size = {512.0f, 768.0f};
            ws.position = {width * 0.5f - ws.size.x * 0.5f, height * 0.5f + ws.size.y * 0.5f};
            ws.label = "chat";

            window_state["chat_window"] = ws;
        }
    }


    if(open_settings) {
        bool close_window = false;
        window("settings_window", close_window);

        if(close_window) {
            open_settings = false;
        }
    }

    if(open_stats) {
        bool close_window = false;
        window("stats_window", close_window);

        std::string pos_text = to_base(ct.position.x, 16, 3) + " " + to_base(ct.position.y, 16, 3);
        text(pos_text);

        if(close_window) {
            open_stats = false;
        }
    }

    if(open_chat) {
        bool close_window = false;
        window("chat_window", close_window);

        if(close_window) {
            open_chat = false;
        }
    }
}

std::vector<UI_vertex> create_char(Glyph_data& glyph, vec2 origin) {
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
        v.pos = origin + v.pos * vec2(glyph.size[0], glyph.size[1]);
        v.tex_pos = vec2(glyph.pos_tex[0], glyph.pos_tex[1]) + v.tex_pos * vec2(glyph.size[0], glyph.size[1]);
    }

    return ret;
}

std::vector<UI_vertex> mesh_text(Font& f, std::string text) {
    std::vector<UI_vertex> ret;
    vec2 pos = vec2(0.0f);

    for(char c : text) {
        Glyph_data& gd = f.at(c);

        if(gd.visible) {
            std::vector<UI_vertex> vs = create_char(gd, pos);

            ret.insert(ret.end(), vs.begin(), vs.end());
        }

        pos.x += float(gd.stride);
    }

    return ret;
}

bool contains(vec2 pos, ivec4 range) {
    return pos.x >= range.x && pos.x < range.z && pos.y >= range.y && pos.y < range.w;
}

void GUI_system::window(std::string name, bool& close_window) {
    Window_state& ws = window_state[name];
    float header = 48.0f;
    vec2& position = ws.position;
    vec2& size = ws.size;

    Input_system& input_system = ecs.get_system<Input_system>();

    int buffer = 16.0f;
    ivec4 buffer_range = ivec4(-buffer, -buffer, buffer, buffer);

    ivec4 range_move = {position + vec2(0, -header), position + vec2(size.x, 0)};
    range_move += buffer_range;

    buffer = 24.0f;
    buffer_range = ivec4(-buffer, -buffer, buffer, buffer);

    ivec4 range_left = {position + vec2(0, -header - size.y), position + vec2(0, -header)};
    ivec4 range_right = {position + vec2(size.x, -header - size.y), position + vec2(size.x, -header)};
    ivec4 range_bottom = {position + vec2(0, -header - size.y), position + vec2(size.x, -header - size.y)};

    ivec4 range_close = {position + vec2(size.x - header, -header), position + vec2(size.x, 0.0f)};

    range_left += buffer_range;
    range_right += buffer_range;
    range_bottom += buffer_range;

    vec2 min_size = vec2(256, 128);

    if(capture_id != 0xFFFFFFFF && capture_widget == name) {
        Pointer& pointer = input_system.pointers.at(capture_id);

        float delta_min;

        switch(capture_operation) {
            case 0:
                position += pointer.pos - pointer.prev_pos;
                break;
            case 1:
                position.x += pointer.pos.x - pointer.prev_pos.x;
                size.x -= pointer.pos.x - pointer.prev_pos.x;

                if(pointer.pos.x + buffer > position.x) {
                    float delta_max = (pointer.pos.x + buffer) - position.x;
                    position.x += delta_max;
                    size.x -= delta_max;
                }

                delta_min = max(0.0f, min_size.x - size.x);
                size.x += delta_min;
                position.x -= delta_min;

                break;
            case 2:
                size.x += pointer.pos.x - pointer.prev_pos.x;

                if(pointer.pos.x - buffer < position.x + size.x) {
                    float delta_max = (position.x + size.x) - (pointer.pos.x - buffer);
                    size.x -= delta_max;
                }

                delta_min = max(0.0f, min_size.x - size.x);
                size.x += delta_min;

                break;
            case 3:
                size.y -= pointer.pos.y - pointer.prev_pos.y;

                if(pointer.pos.y - buffer > position.y - size.y - header) {
                    float delta_max = (pointer.pos.y - buffer) - (position.y - size.y - header);
                    size.y -= delta_max;
                }

                delta_min = max(0.0f, min_size.y - size.y);
                size.y += delta_min;

                break;
            case 4:
                // x axis
                position.x += pointer.pos.x - pointer.prev_pos.x;
                size.x -= pointer.pos.x - pointer.prev_pos.x;

                if(pointer.pos.x + buffer > position.x) {
                    float delta_max = (pointer.pos.x + buffer) - position.x;
                    position.x += delta_max;
                    size.x -= delta_max;
                }

                delta_min = max(0.0f, min_size.x - size.x);
                size.x += delta_min;
                position.x -= delta_min;

                // y axis
                size.y -= pointer.pos.y - pointer.prev_pos.y;

                if(pointer.pos.y - buffer > position.y - size.y - header) {
                    float delta_max = (pointer.pos.y - buffer) - (position.y - size.y - header);
                    size.y -= delta_max;
                }

                delta_min = max(0.0f, min_size.y - size.y);
                size.y += delta_min;

                break;
            case 5:
                // x axis
                size.x += pointer.pos.x - pointer.prev_pos.x;

                if(pointer.pos.x - buffer < position.x + size.x) {
                    float delta_max = (position.x + size.x) - (pointer.pos.x - buffer);
                    size.x -= delta_max;
                }

                delta_min = max(0.0f, min_size.x - size.x);
                size.x += delta_min;

                // y axis
                size.y -= pointer.pos.y - pointer.prev_pos.y;

                if(pointer.pos.y - buffer > position.y - size.y - header) {
                    float delta_max = (pointer.pos.y - buffer) - (position.y - size.y - header);
                    size.y -= delta_max;
                }

                delta_min = max(0.0f, min_size.y - size.y);
                size.y += delta_min;

                break;
        }
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
                    close_window = true;
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

    vec4 header_range = vec4(position + vec2(0.0f, -header), position + vec2(size.x, 0.0f));

    std::vector<UI_vertex> total_ret;

    UI_vertex a = {vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec4(1.0f)};
    UI_vertex b = {vec2(1.0f, 0.0f), vec2(1.0f, 0.0f), vec4(1.0f)};
    UI_vertex c = {vec2(0.0f, 1.0f), vec2(0.0f, 1.0f), vec4(1.0f)};
    UI_vertex d = {vec2(1.0f, 1.0f), vec2(1.0f, 1.0f), vec4(1.0f)};

    std::vector<UI_vertex> ret = {a, b, d, a, d, c};
    for(UI_vertex& v : ret) {
        v.pos = position + vec2(0.0, -size.y - header) + v.pos * size;
        v.tex_pos = vec2(1.0f);
        v.color = vec4(0.35f, 0.35f, 0.35f, 0.35f);
        v.data = 1;
    }
    total_ret.insert(total_ret.end(), ret.begin(), ret.end());

    ret = {a, b, d, a, d, c};
    for(UI_vertex& v : ret) {
        v.pos = position + vec2(0.0, -header) + v.pos * vec2(size.x, header);
        v.tex_pos = vec2(1.0f);
        v.color = vec4(1.0f, 0.35f, 0.35f, 1.0f);
        v.data = 1;
    }
    total_ret.insert(total_ret.end(), ret.begin(), ret.end());

    ret = mesh_text(fonts["default_mono"], ws.label);

    uint32_t tex_scale = 3.0f;

    for(UI_vertex& v : ret) {
        float s = floor(header * 0.5f - 11.0f * float(tex_scale) * 0.5f);
        v.pos = position + v.pos * float(tex_scale) + vec2(s, -s - 11.0f * float(tex_scale));
        v.range = header_range;
    }
    total_ret.insert(total_ret.end(), ret.begin(), ret.end());

    vec4 range = vec4(0, 2, 5, 7);

    ret = {a, b, d, a, d, c};
    for(UI_vertex& v : ret) {
        float s = floor(header * 0.5f - 5.0f * float(tex_scale) * 0.5f);
        vec2 icon_size = vec2(range.z - range.x, range.w - range.y);

        v.pos = position + v.pos * float(tex_scale) * icon_size + vec2(size.x - s - 5.0f * float(tex_scale), -s - 5.0f * float(tex_scale));
        v.tex_pos = v.tex_pos * icon_size + vec2(range.x, range.y);
        v.color = vec4(1.0f);
        v.data = 1;
        v.range = header_range;
    }
    total_ret.insert(total_ret.end(), ret.begin(), ret.end());

    vertices.insert(vertices.end(), total_ret.begin(), total_ret.end());

    current_pos = position + vec2(0.0f, -header) + vec2(widget_buffer,  -widget_buffer);
    current_range = vec4(position + vec2(0.0f, -header - size.y), position + vec2(size.x, -header));
}

void GUI_system::end_window() {

}

void GUI_system::tab(vec2 position, vec2 size, ivec4 icon, bool& active) {
    Input_system& input_system = ecs.get_system<Input_system>();

    std::vector<UI_vertex> total_ret;

    UI_vertex a = {vec2(0.0f, 0.0f), vec2(0.0f, 0.0f), vec4(1.0f)};
    UI_vertex b = {vec2(1.0f, 0.0f), vec2(1.0f, 0.0f), vec4(1.0f)};
    UI_vertex c = {vec2(0.0f, 1.0f), vec2(0.0f, 1.0f), vec4(1.0f)};
    UI_vertex d = {vec2(1.0f, 1.0f), vec2(1.0f, 1.0f), vec4(1.0f)};

    std::vector<UI_vertex> ret = {a, b, d, a, d, c};
    for(UI_vertex& v : ret) {
        v.pos = position + v.pos * size;
        v.tex_pos = vec2(1.0f);
        if(active) v.color = vec4(0.65f);
        else v.color = vec4(0.35f, 0.35f, 0.35f, 0.35f);
        v.data = 1;
    }
    total_ret.insert(total_ret.end(), ret.begin(), ret.end());

    vec2 size_tex = vec2(icon.z - icon.x, icon.w - icon.y);
    float scale_tex = 6.0f;

    float buffer = floor((size.y - size_tex.y * scale_tex) * 0.5f);

    ret = {a, b, d, a, d, c};
    for(UI_vertex& v : ret) {
        v.pos = position + vec2(size.x - buffer - size_tex.x * scale_tex, buffer) + v.pos * size_tex * scale_tex;
        v.tex_pos = v.tex_pos * size_tex + vec2(icon.x, icon.y);
        v.color = vec4(1.0f);
        v.data = 1;
    }
    total_ret.insert(total_ret.end(), ret.begin(), ret.end());

    vertices.insert(vertices.end(), total_ret.begin(), total_ret.end());

    ivec4 range = ivec4(position, position + size);
    if(capture_id == 0xFFFFFFFF) {
        for(auto &[key, pointer]: input_system.pointers) {
            if(pointer.down < 2) {
                if(contains(pointer.pos, range)) {
                    capture_id = key;
                    capture_operation = 10;
                    active = !active;
                }
            }
        }
    }

}

void GUI_system::text(std::string text) {
    uint32_t tex_scale = 3.0f;

    std::vector<UI_vertex> ret = mesh_text(fonts["default_mono"], text);

    for(UI_vertex& v : ret) {
        v.pos = current_pos + v.pos * float(tex_scale) + vec2(0.0f, -11.0f * float(tex_scale));
        v.range = current_range;
    }

    vertices.insert(vertices.end(), ret.begin(), ret.end());

    current_pos += vec2(0.0f, -11.0f * float(tex_scale) - widget_buffer);
}
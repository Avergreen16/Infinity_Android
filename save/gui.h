//
// Created by plane on 2/14/2026.
//

#pragma once

#include <map>
#include <fstream>

#include "ecs.h"
#include "logging.h"
#include "wrapper.h"

const std::string integers = "0123456789\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89";
const std::string integers_letters = "0123456789ABCDEF";

enum cursor_mode{CURSOR_CLICK, CURSOR_RESIZE_T, CURSOR_RESIZE_TR, CURSOR_RESIZE_R, CURSOR_RESIZE_BR, CURSOR_RESIZE_B, CURSOR_RESIZE_BL, CURSOR_RESIZE_L, CURSOR_RESIZE_TL, CURSOR_TEXT};
enum panel_split{SPLIT_X, SPLIT_Y, SPLIT_LEAF};

struct UI_vertex {
    vec2 pos;
    vec2 tex_pos;
    vec4 color = vec4(1.0f);
    vec4 range = vec4(-FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX);
    uint data = 0;
};

// font

struct Glyph_data {
    bool visible;
    uint8_t stride;

    std::array<uint8_t, 2> size;
    std::array<uint16_t, 2> pos_tex;
    std::array<int8_t, 2> pos_line;
};

struct Font {
    uint8_t line_height;
    Glyph_data empty_data = {false, 0, 0, 0};
    std::map<char, Glyph_data> glyph_map;

    Glyph_data& at(char key) {
        if(glyph_map.find(key) != glyph_map.end()) return glyph_map[key];
        return empty_data;
    }

    void init(std::string filepath) {
        std::vector<uint8_t> bytes = get_bytes_from_file(filepath);
        uint32_t pos = 0;

        auto read = [&](void* ptr, uint32_t num_bytes) {
            memcpy(ptr, &bytes[pos], num_bytes);
            pos += num_bytes;
        };

        read(&line_height, 1);

        uint16_t invisible_glyphs;
        read(&invisible_glyphs, 2);

        for(int i = 0; i < invisible_glyphs; ++i) {
            uint8_t id;
            Glyph_data data;
            data.visible = false;

            read(&id, 1);
            read(&data.stride, 1);

            glyph_map.insert({id, data});
        }

        uint16_t visible_glyphs;
        read(&visible_glyphs, 2);

        for(int i = 0; i < visible_glyphs; ++i) {
            uint8_t id;
            Glyph_data data;
            data.visible = true;

            read(&id, 1);
            read(&data.stride, 1);
            read(&data.size[0], 1);
            read(&data.size[1], 1);
            read(&data.pos_tex[0], 2);
            read(&data.pos_tex[1], 2);
            read(&data.pos_line[0], 1);
            read(&data.pos_line[1], 1);

            glyph_map.insert({id, data});
        }

        // missing placeholder
        Glyph_data data;
        data.visible = true;
        read(&data.stride, 1);
        read(&data.size[0], 1);
        read(&data.size[1], 1);
        read(&data.pos_tex[0], 2);
        read(&data.pos_tex[1], 2);
        read(&data.pos_line[0], 1);
        read(&data.pos_line[1], 1);

        empty_data = data;
    }

    Font(std::string filepath) {
        init(filepath);
    }

    Font() = default;
    Font(const Font& f) = default;
    Font(Font&& f) = default;
};

// gui system

enum LAYOUT_MODE{LM_VOID, LM_ROW, LM_COLUMN, LM_GRID};
enum POSITION_MODE{PM_STATIC, PM_TOP_LEFT, PM_TOP_RIGHT, PM_BOTTOM_LEFT, PM_BOTTOM_RIGHT, PM_TOP_CENTER, PM_BOTTOM_CENTER, PM_CENTER_LEFT, PM_CENTER_RIGHT, PM_CENTER, PM_VOID};
enum SIZE_MODE{SM_STATIC, SM_FILL, SM_SURROUND};
enum TEXT_ALIGN{TE_LEFT, TE_CENTER, TE_RIGHT};

struct Widget {
    vec2 position;
    vec2 size;
    std::vector<int> data;

    LAYOUT_MODE layout_mode = LM_VOID;
    POSITION_MODE position_mode = PM_STATIC;
    SIZE_MODE size_mode = SM_STATIC;

    vec2 buffer = vec2(0.0f);
    vec4 range = vec4(-FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX);

    vec2 child_offset = vec2(0.0f);

    uint32_t parent = NULL_ENTITY;
    std::vector<uint32_t> children;
    std::vector<UI_vertex> vertices;

    std::function<void(Widget&)> size_func = [](Widget&) {};
    std::function<void(Widget&)> input_func = [](Widget&) {};
    std::function<void(Widget&)> mesh_func = [](Widget&) {};
};

struct Window_state {
    vec2 position;
    vec2 size;
    float scroll = 0.0f;
    std::string label;

    bool active = false;
};

struct GUI_system : System {
    std::unordered_map<std::string, Font> fonts;
    std::vector<UI_vertex> vertices;

    std::unordered_map<std::string, Window_state> window_state;
    std::vector<Widget> widgets;
    std::vector<uint32_t> widget_path = {0};

    uint32_t capture_id = 0xFFFFFFFF;
    std::string capture_widget;
    uint32_t capture_operation;

    vec4 current_range = vec4(-FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX);

    vec2 widget_buffer = vec2(0.0f);
    POSITION_MODE position_mode = PM_TOP_LEFT;

    bool measure = true;

    GUI_system();
    ~GUI_system();
    void init();
    void call();

    void ui_loop();

    void debug_square(vec2 size, vec4 color, bool fill = false);
    void button(vec2 size, ivec4 icon, vec4 color, std::function<void()> on_click);
    void window(std::string name, Window_state state, std::function<void()> on_close);
    void text(std::string text);

    void row();
    void column();
    void grid(ivec2 size);

    void set_mode(POSITION_MODE mode);
    void set_buffer(vec2 buffer);
    void reset();
    void step();
};

float get_text_y(Font& f, std::string text, uint32_t text_size, uint32_t width = 0xFFFFFFFF, TEXT_ALIGN alignment = TE_LEFT);
std::vector<UI_vertex> mesh_text(Font& f, std::string text, uint32_t text_size, uint32_t width = 0xFFFFFFFF, TEXT_ALIGN alignment = TE_LEFT);

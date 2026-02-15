//
// Created by plane on 2/14/2026.
//

#pragma once

#include <map>
#include <fstream>

#include "ecs.h"
#include "logging.h"
#include "wrapper.h"

struct UI_vertex {
    vec2 pos;
    vec2 tex_pos;
    vec4 color;
    vec4 range = vec4(-FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX);
    uint data = 0;
};

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

struct Window_state {
    vec2 position;
    vec2 size;
    std::string label;

};

struct GUI_system : System {
    std::unordered_map<std::string, Font> fonts;
    std::vector<UI_vertex> vertices;

    std::unordered_map<std::string, Window_state> window_state;

    bool init_gui = false;
    uint32_t capture_id = 0xFFFFFFFF;
    std::string capture_widget;
    uint32_t capture_operation;

    vec2 current_pos;
    float widget_buffer = 24.0f;
    vec4 current_range = vec4(-FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX);

    GUI_system();
    ~GUI_system();
    void init();
    void call();

    void tab(vec2 position, vec2 size, ivec4 icon, bool& active);
    void window(std::string name, bool& close_window);
    void end_window();
    void text(std::string text);
};

std::vector<UI_vertex> mesh_text(Font& f, std::string text);

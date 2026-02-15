#version 320 es

precision mediump float;

layout(binding = 0) uniform sampler2D text_tex;
layout(binding = 1) uniform sampler2D gui_tex;

layout(location = 0) out vec4 frag_color;

layout(location = 0) in vec2 tex_coord;
layout(location = 1) in vec4 color;
layout(location = 2) flat in uint data;
layout(location = 3) in vec4 range;
layout(location = 4) in vec2 pos;

void main() {
    if(pos.x < range.x || pos.y < range.y || pos.x >= range.z || pos.y >= range.w) discard;

    if(data == 0u) {
        frag_color = texture(text_tex, tex_coord / vec2(611.0f, 11.0f)) * color;
    } else {
        frag_color = texture(gui_tex, tex_coord / vec2(64.0f, 64.0f)) * color;
    }
}
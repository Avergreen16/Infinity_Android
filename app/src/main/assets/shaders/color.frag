#version 320 es

precision mediump float;

layout(binding = 0) uniform sampler2D grid_tex;

layout(location = 3) uniform vec4 color;

layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec4 frag_normal;

layout(location = 0) in vec2 tex_coord;

void main() {
    frag_color = vec4(color) * texture(grid_tex, tex_coord);
    frag_normal = vec4(0.0, 0.0, 0.0, 1.0);
}
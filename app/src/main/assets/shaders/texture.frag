#version 320 es

precision mediump float;

layout(binding = 0) uniform sampler2D color_tex;

layout(location = 3) uniform vec4 color;

layout(location = 0) in vec2 tex_coord;

layout(location = 0) out vec4 frag_color;

void main() {
    frag_color = texture(color_tex, tex_coord / vec2(textureSize(color_tex, 0)));
}
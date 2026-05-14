#version 320 es

precision highp float;
precision highp int;

layout(binding = 0) uniform sampler2D color_tex;

layout(location = 2) uniform mat4 model;
layout(location = 5) uniform vec4 range;

layout(location = 0) in vec2 pos;

layout(location = 0) out vec4 frag_color;

void main() {
    vec2 p = pos * range.zw + range.xy;

    vec4 color = texture(color_tex, p);

    frag_color = color;
}
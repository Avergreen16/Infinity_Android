#version 320 es

precision mediump float;

layout(location = 0) in vec3 normal;

layout(location = 3) uniform vec4 color;
layout(location = 4) uniform vec3 light;

layout(location = 0) out vec4 frag_color;

void main() {
    frag_color = vec4(color);
    frag_color.xyz *= dot(normal, light);
}
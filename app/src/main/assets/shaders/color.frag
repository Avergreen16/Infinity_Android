#version 320 es

precision mediump float;

layout(location = 3) uniform vec4 color;

layout(location = 0) out vec4 frag_color;

void main() {
    frag_color = vec4(color);
}
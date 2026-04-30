#version 320 es

precision highp float;
precision highp int;

layout(binding = 0) uniform sampler2D screen_tex;

layout(location = 0) in vec2 pos;

layout(location = 0) out vec4 frag_color;

void main() {
    frag_color = texture(screen_tex, pos * 0.5f + 0.5f);
}
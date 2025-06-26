#version 320 es

layout(location = 0) in vec3 position;

layout(location = 0) uniform mat4 proj;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform mat4 model;

layout(location = 0) out vec2 tex_coord;

void main() {
    gl_Position = view * model * vec4(position, 1.0);
    tex_coord = position.xz * 5.0f;

    gl_Position = proj * gl_Position;
}
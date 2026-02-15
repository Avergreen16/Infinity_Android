#version 320 es

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 tex_coord;
layout(location = 2) in vec4 color;
layout(location = 3) in vec4 range;
layout(location = 4) in uint data;

layout(location = 0) uniform mat4 proj;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform mat4 model;

layout(location = 0) out vec2 tc;
layout(location = 1) out vec4 _color;
layout(location = 2) flat out uint _data;
layout(location = 3) out vec4 _range;
layout(location = 4) out vec2 _pos;

void main() {
    gl_Position = view * model * vec4(position, 0.6f, 1.0);
    tc = tex_coord;

    gl_Position = proj * gl_Position;

    _color = color;
    _data = data;
    _range = range;
    _pos = position;
}
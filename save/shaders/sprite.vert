#version 320 es

precision highp float;

layout(location = 0) uniform mat4 proj;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform mat4 model;
layout(location = 3) uniform vec2 size;
layout(location = 4) uniform vec2 offset;

layout(location = 0) out vec2 pos;

void main() {
    vec2 vs[6] = vec2[6](
        vec2(0.0, 0.0),
        vec2(1.0, 0.0),
        vec2(1.0, 1.0),
        vec2(0.0, 0.0),
        vec2(1.0, 1.0),
        vec2(0.0, 1.0)
    );


    gl_Position = proj * view * model * vec4(vs[gl_VertexID] * size + offset, 0.5, 1.0);

    pos = vs[gl_VertexID];
}
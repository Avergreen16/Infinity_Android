#version 320 es

precision highp float;

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


    gl_Position = vec4(vs[gl_VertexID] * 2.0f - 1.0f, 0.5, 1.0);

    pos = vs[gl_VertexID] * 2.0f - 1.0f;
}
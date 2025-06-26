#version 320 es

vec3 grid_plane[6] = vec3[](
vec3(1, 1, 0), vec3(-1, 1, 0), vec3(-1, -1, 0),
vec3(-1, -1, 0), vec3(1, -1, 0), vec3(1, 1, 0)
);

precision mediump float;

layout(location = 0) uniform mat4 proj;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform mat4 model;

layout(location = 0) out vec2 tex_coord;
layout(location = 1) out mat4 inverse_proj;
layout(location = 2) out mat4 inverse_model;

void main() {
    tex_coord = grid_plane[gl_VertexID].xy;
    gl_Position = vec4(grid_plane[gl_VertexID].xyz, 1.0);

    inverse_proj = inverse(proj);
    inverse_model = -model;
}
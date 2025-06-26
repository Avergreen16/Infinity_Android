#version 320 es

precision mediump float;

layout(location = 0) uniform mat4 proj;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform mat4 model;

layout(location = 0) in vec2 tex_coord;
layout(location = 1) in mat4 inverse_proj;
layout(location = 2) in mat4 inverse_model;

out vec4 frag_color;

float ray_plane(vec3 dir, vec3 pos, vec3 plane_pos, vec3 plane_normal) {
    float x = -dot(pos - plane_pos, plane_normal) / dot(plane_normal, dir);

    return x;
}

float map(float v, float min1, float max1, float min2, float max2) {
    float i = (v - min1) / (max1 - min1);

    return i * (max2 - min2) + min2;
}

vec4 get_color(vec2 pos, vec3 camera_pos, vec3 view_dir, mat3 norm) {
    vec4 color = vec4(0.0);

    float lw = 1.0 / 32.0;
    uint num_scales = uint(4);
    float width = 1.0;
    int starting_scale = max(0, int(floor(log(abs(camera_pos.z)) / log(16.0) - 0.5)));

    float bounds = pow(2.0, 71.0);

    int min_s = 0;
    int max_s = int(floor(log(bounds) / log(16.0)));

    for(int i = 2; i >= -1; --i) {
        int ii = i + starting_scale;
        if(ii >= min_s && ii <= max_s) {
            vec4 x_color = vec4(0.0);
            vec4 y_color = vec4(0.0);

            bool skip = false;

            float radius = pow(16.0, float(ii));

            vec2 tex_pos = pos / radius;
            vec2 dx = dFdx(tex_pos);
            vec2 dy = dFdy(tex_pos);
            vec2 ax = vec2(dx.x, dy.x);
            vec2 ay = vec2(dx.y, dy.y);
            float ddx = length(ax);
            float ddy = length(ay);

            float fade1 = min(abs(camera_pos.z) / (radius * 0.5), 1.0);
            if(ii == 0) fade1 = 1.0;

            float line_width = max(lw * fade1, ddy);
            float fade = 1.0;
            if(line_width <= ddy) {
                fade = lw * fade1 / ddy;
            }

            float dd = length(vec3(pos, 0.0) - camera_pos);

            float ddd = -line_width * 0.5 * radius;
            //if(tex_pos.y > 0.0) ddd = -ddd;

            if(abs(pos.y) + ddd <= bounds) {
                tex_pos = pos / radius + line_width * 0.5;

                vec2 mod_tex_pos = mod(tex_pos, 1.0);


                if(mod_tex_pos.y < line_width) {
                    if(tex_pos.y < line_width && tex_pos.y > 0.0 && abs(tex_pos.x - line_width * 0.5) > abs(tex_pos.y - line_width * 0.5)) {
                        if(tex_pos.x - line_width * 0.5 > 0.0) x_color = vec4(1.0, 0.25, 0.25, 2.0);
                        else x_color = vec4(0.25, 1.0, 1.0, 2.0);
                    } else x_color = vec4(1.0, 1.0, 1.0, fade);
                }
            } else skip = true;



            line_width = max(lw * fade1, ddx);
            fade = 1.0;
            if(line_width <= ddx) {
                fade = lw * fade1 / ddx;
            }

            ddd = -line_width * 0.5 * radius;
            //if(tex_pos.x > 0.0) ddd = -ddd;

            if(abs(pos.x) + ddd <= bounds) {
                tex_pos = pos / radius + line_width * 0.5;
                vec2 mod_tex_pos = mod(tex_pos, 1.0);


                if(mod_tex_pos.x < line_width) {
                    if(tex_pos.x < line_width && tex_pos.x > 0.0 && abs(tex_pos.y - line_width * 0.5) > abs(tex_pos.x - line_width * 0.5)) {
                        if(tex_pos.y - line_width * 0.5 > 0.0) y_color = vec4(0.25, 1.0, 0.25, 2.0);
                        else y_color = vec4(1.0, 0.25, 1.0, 2.0);
                    } else y_color = vec4(1.0, 1.0, 1.0, fade);
                }
            } else skip = true;

            vec3 dir = vec3(pos, 0.0) - camera_pos;
            float dist = length(dir);

            if(!skip) {
                if(color.w < x_color.w) color = x_color;
                if(color.w < y_color.w) color = y_color;
            }

            //color = vec4(color.xyz * (1.0 - add_color.w) + add_color.xyz * add_color.w, max(color.w, add_color.w));
        }
    }

    color.w = min(1.0, color.w);

    return color;
}

uint hash(uint x) {
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}
float to_float(uint m) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat(m);       // Range [1:2]
    return f - 1.0;                // Range [0:1]
}

mat3 y_norm = mat3(
vec3(1, 0, 0),
vec3(0, 0, 1),
vec3(0, 1, 0)
);

mat3 x_norm = mat3(
vec3(0, 0, 1),
vec3(0, 1, 0),
vec3(1, 0, 0)
);

mat3 z_norm = mat3(
vec3(1, 0, 0),
vec3(0, 1, 0),
vec3(0, 0, 1)
);

void main() {
    mat3 norm_mat = z_norm;

    vec4 v = vec4(tex_coord, 0.5, 1.0);
    v = inverse_proj * v;
    v /= v.w;
    v = vec4(normalize(v.xyz), 1.0);
    vec3 rel_v = v.xyz;

    v = transpose(view) * v;
    vec3 pos = vec3(inverse_model * vec4(0.0, 0.0, 0.0, 1.0));

    float x = ray_plane(v.xyz, pos, vec3(0.0), norm_mat * vec3(0.0, 0.0, 1.0));

    float d = dot(rel_v, vec3(0, 0, 1));

    if(x <= 0.0) discard;

    vec4 p = vec4(0.0, 0.0, x * d, 1.0);

    p = proj * p;

    p /= p.w;

    vec3 plane_pos = (pos + v.xyz * x);
    vec3 view_dir = vec3(transpose(view) * vec4(0, 0, -1, 1));


    plane_pos = norm_mat * plane_pos;
    view_dir = norm_mat * view_dir;
    pos = norm_mat * pos;

    vec4 color = get_color(plane_pos.xy, pos, view_dir, norm_mat);

    if(color.w == 0.0) discard;

    gl_FragDepth = p.z;

    frag_color = color;
}
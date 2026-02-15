#version 320 es

precision highp float;
precision highp int;

layout(location = 0) out vec4 frag_color;

layout(location = 0) in vec2 coords2;

layout(location = 2) uniform vec2 screen_size;

vec3 hex_color(uint i) {
    return vec3((i >> 16) & 0xFFu, (i >> 8) & 0xFFu, i & 0xFFu) / float(0xFFu);
}

void main() {
    vec2 coords = coords2;

    vec3 base_color = hex_color(0x1E1F2Eu);
    vec3 x_color = hex_color(0xFF4F4Fu);
    vec3 nx_color = hex_color(0x4FFFFFu);
    vec3 y_color = hex_color(0x4FFF4Fu);
    vec3 ny_color = hex_color(0xFF4FFFu);

    vec4 c = vec4(base_color, 1.0);

    int line_width = 1;


    vec2 dx = dFdx(coords);
    //vec2 dy = dFdy(coords);

    float ddx = fwidth(coords.x);
    float ddy = fwidth(coords.y);

    vec4 line_c = vec4(0.0);
    vec4 line_a = vec4(0.0);

    float width_x = length(dx) * screen_size.x;

    int s = int(round(log2(width_x) * 0.25));

    ivec2 power_scales = ivec2(max(-1, s - 2), s + 1);

    for(int i = power_scales.x; i <= power_scales.y; ++i) {
        float grid_width = pow(16.0, float(i));

        float dist_from_line = abs(fract(float(coords.x / grid_width) + 0.5f) - 0.5f) * grid_width;
        float dist_from_center = abs(coords.x);

        float alpha = clamp(1.0 - (ddx / (grid_width * 0.125f)), 0.0f, 1.0f);
        if(dist_from_line < ddx) {
            if(dist_from_center < ddx) {
                if(coords.y > 0.0f) line_a = max(line_a, vec4(y_color, 1.0f));
                else line_a = max(line_a, vec4(ny_color, 1.0f));
            }
            line_c = max(line_c, vec4(base_color * 2.0f, alpha));
        }

        dist_from_line = abs(fract(float(coords.y / grid_width) + 0.5f) - 0.5f) * grid_width;
        dist_from_center = abs(coords.y);

        alpha = clamp(1.0 - (ddy / (grid_width * 0.125f)), 0.0f, 1.0f);
        if(dist_from_line < ddy) {
            if(dist_from_center < ddy) {
                if(coords.x > 0.0f) line_a = max(line_a, vec4(x_color, 1.0f));
                else line_a = max(line_a, vec4(nx_color, 1.0f));
            }
            line_c = max(line_c, vec4(base_color * 2.0f, alpha));
        }
    }

    c.rgb = line_c.rgb * line_c.w + c.rgb * (1.0 - line_c.w);
    c.rgb = line_a.rgb * line_a.w + c.rgb * (1.0 - line_a.w);

    frag_color = c;
}
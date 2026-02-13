#version 320 es

precision mediump float;

layout(location = 0) out vec4 frag_color;

layout(location = 0) in vec2 coords;

vec3 hex_color(uint i) {
    return vec3((i >> 16) & uint(0xFF), (i >> 8) & uint(0xFF), i & uint(0xFF)) / float(0xFF);
}

void main() {
    /*
    vec3 base_color = hex_color(uint(0x1E1F2E));
    vec3 sector_color = hex_color(uint(0xFF893D));

    vec4 c = vec4(base_color, 1.0);

    int line_width = 1;

    vec2 dx = dFdx(coords);
    vec2 dy = dFdy(coords);

    float ddx = length(vec2(dx.x, dy.x));
    float ddy = length(vec2(dx.y, dy.y));

    vec4 line_c = vec4(0.0);
    vec4 line_a = vec4(0.0);


    // x direction
    bool flag = false;
    vec3 color = base_color * 2.0;
    if(int(round(coords.x)) % 256 == 0) {
        color = sector_color;
        line_width = 2;
        flag = true;
    }
    if(int(round(coords.x)) == 0) {
        color = hex_color(uint(0xFF5959));
        line_width = 2;
        flag = true;
    }

    float dist_from_line = abs(fract(float(coords.x) + 0.5) - 0.5) / float(line_width) * 2.0;
    float alpha = max(0.0, 1.0 - (ddx / (float(line_width) * float(line_width))) * 2.0);
    if(dist_from_line < ddx) {
        if(flag) line_a = max(line_a, vec4(color, alpha));
        else {
            line_c = vec4(color, alpha);
        }
    }

    // x direction pixel
    color = base_color * 2.0;
    line_width = 1;
    dist_from_line = abs(fract(coords.x * 16.0 + 0.5) - 0.5) / 16.0 / float(line_width) * 2.0;
    alpha = max(0.0, 1.0 - (ddx * 16.0 / float(line_width)) * 2.0);
    if(dist_from_line < ddx) {
        line_c = max(line_c, vec4(color, alpha * 0.5));
    }

    // y direction
    flag = false;
    line_width = 1;
    color = base_color * 2.0;
    if(int(round(coords.y)) % 256 == 0) {
        color = sector_color;
        line_width = 2;
        flag = true;
    }
    if(int(round(coords.y)) == 0) {
        color = hex_color(uint(0x59FF59));
        line_width = 2;
        flag = true;
    }

    dist_from_line = abs(fract(coords.y + 0.5) - 0.5) / float(line_width) * 2.0;
    alpha = max(0.0, 1.0 - (ddy / float(line_width)) * 2.0);
    if(dist_from_line < ddy) {
        if(flag) line_a = max(line_a, vec4(color, alpha));
        else line_c = vec4(color, alpha);
    }

    // y direction pixel
    color = base_color * 2.0;
    line_width = 1;
    dist_from_line = abs(fract(coords.y * 16.0 + 0.5) - 0.5) / 16.0 / float(line_width) * 2.0;
    alpha = max(0.0, 1.0 - (ddy * 16.0 / float(line_width)) * 2.0);
    if(dist_from_line < ddy) {
        line_c = max(line_c, vec4(color, alpha * 0.5));
    }

    c.rgb = line_c.rgb * line_c.w + c.rgb * (1.0 - line_c.w);
    c.rgb = line_a.rgb * line_a.w + c.rgb * (1.0 - line_a.w);

    frag_color = c;
    */

    frag_color = vec4(fract(coords), 0.0, 1.0);
}
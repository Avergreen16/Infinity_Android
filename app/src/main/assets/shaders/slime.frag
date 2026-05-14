#version 320 es

precision highp float;
precision highp int;

layout(location = 0) in vec2 pos;

layout(location = 0) out vec4 frag_color;

float sdf_sphere(vec3 p, float radius) {
    return length(p) - radius;
}

vec3 a = vec3(1.0f, 0.0f, 0.0f);
vec3 b = vec3(0.0f, 1.0f, 0.0f);
vec3 c = vec3(0.0f, 0.0f, 1.0f);

void main() {
    vec3 direction = vec3(0.0f, 0.0f, 1.0f);
    vec3 position = vec3(pos * 2.0f, -2.0f);
    vec3 light = normalize(vec3(1.0f, 1.0f, -1.0f));

    int max_steps = 256;
    float len = 4.0f;

    float dist = 0.0f;
    float min_step = 0.01f;

    bool intersect = false;

    for(int i = 0; i < max_steps; ++i) {
        if(!intersect) {
            vec3 pos = position + direction * dist;

            float sdf = sdf_sphere(pos, 1.0f);

            if(sdf < 0.0f) {
                intersect = true;
            } else {
                float step = max(sdf, min_step);

                dist += step;
            }
        }
    }

    vec3 pp = position + direction * dist;

    float sdf_0 = sdf_sphere(pp, 1.0f);
    float sdf_x = sdf_sphere(pp + vec3(min_step, 0.0f, 0.0f), 1.0f);
    float sdf_y = sdf_sphere(pp + vec3(0.0f, min_step, 0.0f), 1.0f);
    float sdf_z = sdf_sphere(pp + vec3(0.0f, 0.0f, min_step), 1.0f);

    vec3 normal = normalize(vec3(sdf_x - sdf_0, sdf_y - sdf_0, sdf_z - sdf_0));

    vec3 color;

    float d = dot(light, normal);

    if(d > 0.98f) color = a;
    else if(d > -0.1f) color = b;
    else color = c;


    if(length(pos) <= 0.5f) frag_color = vec4(color, 1.0f);
}

/*
int main_steps = 32;
int light_steps = 8;
float len = 4.0f;
float light_len = 2.0f;

float deepest = 0.0f;

float transmittance = 0.0f;

float light_v = 0.0f;

for(int i = 0; i < main_steps; ++i) {
    float pi = (float(i) + 0.5f) / float(main_steps) * len;

    vec3 pos = position + direction * pi;

    float sdf = sdf_sphere(pos, 1.0f) * 10.0f;

    // light

    float light_transmittance = 0.0f;

    for(int j = 0; j < light_steps; ++j) {
        float pj = (float(j) + 0.5f) / float(light_steps) * light_len;

        vec3 pos_j = pos + light * pj;

        float sdf2 = sdf_sphere(pos_j, 1.0f);

        if(sdf2 < 0.0f) light_transmittance += (1.0f / float(light_steps));
    }

    float t = exp(-light_transmittance * 5.0f);

    if(sdf < 0.0f) {
        float tt = -sdf * (1.0f / float(main_steps));
        transmittance += tt;
        light_v += tt * t;
    }
}

float d = floor(-deepest * 2.0f) / 2.0f;

if(length(pos) <= 0.5f) frag_color = vec4(vec3(0.25f, 0.65f, 1.0f) * light_v, 1.0f - exp(-transmittance));
*/
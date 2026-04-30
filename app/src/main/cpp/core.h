#pragma once
#include "pnum.h"
#include "renderer.h"
#include "util.h"
#include "wrapper.h"
#include "random.h"

template<typename type>
type& get(std::unique_ptr<type>& a) {
    return (*a.get());
}

template<typename type_before, typename type_after>
type_after& convert(type_before& b) {
    return *(type_after*)&b;
}

std::ostream& operator<<(std::ostream& c, glm::vec3 v);

float smoothstep(float a);

glm::mat3 create_rot_mat(glm::vec3 forward, glm::vec3 up);

glm::mat3 z_rot(float angle);

glm::mat3 x_rot(float angle);

glm::mat3 y_rot(float angle);

enum event_types:uint16_t{KEY, MOUSE_BUTTON, SCROLL, CURSOR, TEXT};

struct Key_event {
    int key;
    int scancode;
    int action;
};

struct Mouse_button_event {
    int button;
    int action;
};

struct Scroll_event {
    double x;
    double y;
};

struct Cursor_event {
    double xpos;
    double ypos;
};

struct Text_event {
    uint32_t codepoint;
};

using Event = std::variant<Key_event, Mouse_button_event, Scroll_event, Cursor_event, Text_event>;

struct Model;
struct Mesh;

struct Core {
    bool game_running = true;
    
    double start_time = 0;
    double current_time = 0;
    double prev_time = 0;

    std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures;

    /*std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
    std::unordered_map<std::string, std::shared_ptr<Model>> models;
    std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes;*/

    //std::vector<Event> events;

    // matrices
    glm::mat4 proj = glm::identity<mat4>();
    glm::mat4 view = glm::identity<mat4>();

    double delta_time = 1;

    // components
    Random32 random = Random32(0x8FE5006);

    Thread_pool thread_pool;
    
    Profiler profiler;

    Core(int num_threads);

    double get_delta_time();

    glm::mat4 get_infinite_proj_matrix(glm::ivec2 window_size, float fov, float near, float np, float fp);
    glm::mat4 get_proj_matrix(glm::ivec2 window_size, float fov, float near, float far, float np, float fp);

    glm::mat4 get_proj_matrix_ortho(glm::ivec2 window_size, float near, float far, float width, float height);

    glm::mat4 get_view_matrix(glm::vec3 position, glm::vec3 direction, glm::vec3 up);

    glm::mat4 get_model_matrix(pvec3 position, pvec3 origin);

    bool time_step(double step);
};

/*void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void character_callback(GLFWwindow* window, unsigned int codepoint);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);*/

struct aiMesh;
struct aiNode;
struct aiScene;
struct aiAnimation;

extern Core core;
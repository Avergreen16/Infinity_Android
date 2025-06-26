#include "core.h"

glm::mat3 create_rot_mat(glm::vec3 forward, glm::vec3 up) {
    glm::vec3 y = glm::normalize(forward - up * glm::dot(forward, up));
    glm::vec3 x = glm::normalize(glm::cross(y, up));

    return glm::mat3(x, y, up);
}

glm::mat3 z_rot(float angle) {
    glm::vec3 x = glm::vec3(cos(angle), sin(angle), 0);
    glm::vec3 y = glm::vec3(-sin(angle), cos(angle), 0);
    glm::vec3 z = glm::vec3(0, 0, 1);
    return glm::mat3(x, y, z);
}

glm::mat3 x_rot(float angle) {
    glm::vec3 x = glm::vec3(1, 0, 0);
    glm::vec3 y = glm::vec3(0, cos(angle), sin(angle));
    glm::vec3 z = glm::vec3(0, -sin(angle), cos(angle));
    return glm::mat3(x, y, z);
}

glm::mat3 y_rot(float angle) {
    glm::vec3 x = glm::vec3(cos(angle), 0, sin(angle));
    glm::vec3 y = glm::vec3(0, 0, 1);
    glm::vec3 z = glm::vec3(-sin(angle), 0, cos(angle));
    return glm::mat3(x, y, z);
}

/*void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    core.events.push_back(Key_event{key, scancode, action});
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    core.events.push_back(Cursor_event{xpos, ypos});
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    core.events.push_back(Mouse_button_event{button, action});
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    core.events.push_back(Scroll_event{xoffset, yoffset});
}

void character_callback(GLFWwindow* window, unsigned int codepoint) {
    core.events.push_back(Text_event{codepoint});
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    core.window.screen_size.x = width;
    core.window.screen_size.y = height;
    core.window.viewport_size.x = width + 1 * (width & 1);
    core.window.viewport_size.y = height + 1 * (height & 1);

    glViewport(0, 0, core.window.viewport_size.x, core.window.viewport_size.y);

    //core.render();
}


Window::Window(glm::ivec2 size) {
    screen_size = size;
    viewport_size = {size.x + 1 * (size.x & 1), size.y + 1 * (size.y & 1)};
    
    window = glfwCreateWindow(size.x, size.y, "Infinity pre-alpha", NULL, NULL);
    glfwMakeContextCurrent(window);

    if(!gladLoadGL()) {
        std::cout << "ERROR: GLAD failed to load.\n";
        glfwTerminate();
    }

    // init glad and set viewport
    
    glViewport(0, 0, size.x, size.y);
}

void Window::init_callbacks() {
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCharCallback(window, character_callback);
}*/

double Core::get_delta_time() {
    prev_time = current_time;
    current_time = get_time();
    double delta_time = current_time - prev_time;

    return delta_time;
}

std::ostream& operator<<(std::ostream& c, glm::vec3 v) {
    c << v.x << " " << v.y << " " << v.z;

    return c;
}

Core::Core(int num_threads) : thread_pool(num_threads) {}; // : thread_pool(5), thread_pool_b(3) {};

/*glm::mat4 Core::get_proj_matrix(glm::ivec2 window_size, float fov, float near, float far, float np, float fp) {
    float aspect = float(window_size.x) / window_size.y;
    float focal_length = 1.0f / tan(fov * (M_PI / 180) * 0.5f);
    float A = (np * near - fp * far) / (far - near);
    float B = (np - fp) * near * far / (far - near);

    glm::mat4 matrix = glm::identity<glm::mat4>();

    matrix[0][0] = focal_length;
    matrix[1][1] = focal_length * aspect;
    matrix[2][2] = A;
    matrix[3][2] = B;
    matrix[2][3] = -1;

    return matrix;
}*/

glm::mat4 Core::get_infinite_proj_matrix(glm::ivec2 window_size, float fov, float near, float np, float fp) {
    float aspect = float(window_size.x) / window_size.y;
    float focal_length = 1.0f / tan(fov * (M_PI / 180) * 0.5f);
    float A = -fp;
    float B = (np - fp) * near;

    glm::mat4 matrix = glm::identity<glm::mat4>();

    matrix[3][3] = 0;
    matrix[0][0] = focal_length;
    matrix[1][1] = focal_length * aspect;
    matrix[2][2] = A;
    matrix[3][2] = B;
    matrix[2][3] = -1;

    return matrix;
}

glm::mat4 Core::get_proj_matrix_ortho(glm::ivec2 window_size, float near, float far, float width, float height) {
    float right = width * 0.5;
    float left = -width * 0.5;
    float top = height * 0.5;
    float bottom = -height * 0.5;

    mat4 matrix = glm::identity<mat4>();

    matrix[0][0] = 2.0f / (right - left);
    matrix[1][1] = 2.0f / (top - bottom);
    matrix[2][2] = -1.0f / (near - far);
    matrix[3][2] = - (far + near) / (far - near);

    mat4 translate_forward = translate(vec3{0, 0, 0.5f});

    matrix = translate_forward * matrix;

    return matrix;
}

glm::mat4 Core::get_proj_matrix(glm::ivec2 window_size, float fov, float near, float far, float np, float fp) {
    float aspect = float(window_size.x) / window_size.y;
    float focal_length = 1.0f / tan(fov * (M_PI / 180) * 0.5f);
    float A = -(far + near) / (far - near);
    float B = -(2 * far * near) / (far - near);
    ;

    glm::mat4 matrix = glm::identity<glm::mat4>();

    matrix[3][3] = 0;
    matrix[0][0] = focal_length;
    matrix[1][1] = focal_length * aspect;
    matrix[2][2] = A;
    matrix[3][2] = B;
    matrix[2][3] = -1;

    mat4 res_matrix = translate(vec3{0, 0, 0.5f}) * scale(vec3(1.0f, 1.0f, 0.5f));

    matrix = res_matrix * matrix;

    return matrix;
}

glm::mat4 Core::get_view_matrix(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
    return glm::lookAt({0, 0, 0}, direction, up) * glm::inverse(glm::translate(position));
}

glm::mat4 Core::get_model_matrix(pvec3 position, pvec3 origin) {
    vec3 t = position - origin;
    return glm::translate(t);
}

bool Core::time_step(double step) {
    double d_prev = prev_time / step;
    double d_current = current_time / step;

    return floor(d_prev) != floor(d_current);
}

Core core(10);
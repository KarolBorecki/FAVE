#include <cstdint>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <array>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "inc/buffers/vao.h"
#include "inc/buffers/vbo.h"
#include "inc/buffers/ebo.h"

bool first_input_click = true;

struct CoreConfig
{
    uint32_t window_width = 1200;
    uint32_t window_height = 900;
};

struct Camera
{
    glm::vec3 position = glm::vec3(15.71f, 8.86f, 38.01f);
    glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    float fov = 45.0f;
    float near_plane = 0.1f;
    float far_plane = 100.0f;

    float speed = 0.05f;
    float sensitivity = 100.0f;
};

struct GridCell
{
    float p;
    float v, u, w; // v - up/down vec, u - left/right vec, w - forward/backward vec
    float new_v, new_u, new_w;
    float s;
    float new_s;
    float m;
    float new_m;
};

struct Marker
{
    glm::vec3 position;
    glm::vec3 velocity;
    float pressure;
};

struct MacGrid
{
    std::vector<std::vector<GridCell>> cells;
    std::vector<Marker> markers;
    uint16_t size_x;
    uint16_t size_y;
    float cell_size;
};

CoreConfig config;
Camera camera;
MacGrid mac;

GLFWwindow *window;
GLuint shaderProgram;

std::vector<Vertex> vertices = {};
std::vector<GLuint> indices = {};

std::string loadShaderFile(const char *filename);
bool initializeWindow();
GLuint createShaderProgram();
void setupBuffers(VAO &vao);
void processInput();
void transformGridToVerticies();
void render(VAO &vao);
void cleanup();

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    config.window_width = width;
    config.window_height = height;
    glViewport(0, 0, width, height);
}

std::string loadShaderFile(const char *filename)
{
    std::ifstream file(filename);
    if (!file)
    {
        std::cerr << "Error loading shader: " << filename << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool initializeWindow()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(config.window_width, config.window_height, "FLUID SIMULATION", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (!gladLoadGL())
    {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    return true;
}

GLuint createShaderProgram()
{
    std::string vertexSource = loadShaderFile("./resources/shaders/default.vert");
    std::string fragmentSource = loadShaderFile("./resources/shaders/default.frag");

    const char *vertexShaderSource = vertexSource.c_str();
    const char *fragmentShaderSource = fragmentSource.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

void setupBuffers(VAO &vao)
{
    vao.bind();
    VBO vbo(vertices);
    EBO ebo(indices);

    vao.linkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void *)0);
    vao.linkAttrib(vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void *)(3 * sizeof(float)));
    vao.linkAttrib(vbo, 2, 3, GL_FLOAT, sizeof(Vertex), (void *)(6 * sizeof(float)));
    vao.linkAttrib(vbo, 3, 2, GL_FLOAT, sizeof(Vertex), (void *)(9 * sizeof(float)));

    vao.unbind();
    vbo.unbind();
    ebo.unbind();
}

void processInput()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    float camera_speed = camera.speed;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.position += camera_speed * camera.direction;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.position -= camera_speed * camera.direction;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.position -= glm::normalize(glm::cross(camera.direction, camera.up)) * camera_speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.position += glm::normalize(glm::cross(camera.direction, camera.up)) * camera_speed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.position += camera_speed * camera.up;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.position -= camera_speed * camera.up;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        if (first_input_click)
        {
            glfwSetCursorPos(window, config.window_width / 2, config.window_height / 2);
            first_input_click = false;
        }

        double mouse_x, mouse_y;
        glfwGetCursorPos(window, &mouse_x, &mouse_y);

        float rot_x = camera.sensitivity * (float)(mouse_y - (config.window_height / 2)) / config.window_height;
        float rot_y = camera.sensitivity * (float)(mouse_x - (config.window_width / 2)) / config.window_width;

        glm::quat rotation_quat_x = glm::angleAxis(glm::radians(-rot_x), glm::normalize(glm::cross(camera.direction, camera.up)));
        glm::vec3 new_orientation = rotation_quat_x * camera.direction;

        float dot_product = glm::dot(new_orientation, camera.up);
        if (glm::abs(glm::degrees(glm::acos(dot_product)) - 90.0f) <= 85.0f)
        {
            camera.direction = new_orientation;
        }

        glm::quat rotation_quat_y = glm::angleAxis(glm::radians(-rot_y), camera.up);
        camera.direction = rotation_quat_y * camera.direction;

        glfwSetCursorPos(window, config.window_width / 2, config.window_height / 2);
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        first_input_click = true;
    }
}

std::array<float, 3> getSciColor(float val, float minVal, float maxVal)
{
    val = fmin(fmax(val, minVal), maxVal - 0.0001);
    float d = maxVal - minVal;
    val = d == 0.0 ? 0.5 : (val - minVal) / d;
    float m = 0.25;
    uint8_t num = floor(val / m);
    float s = (val - num * m) / m;
    float r, g, b;

    switch (num)
    {
    case 0:
        r = 0.0;
        g = s;
        b = 1.0;
        break;
    case 1:
        r = 0.0;
        g = 1.0;
        b = 1.0 - s;
        break;
    case 2:
        r = s;
        g = 1.0;
        b = 0.0;
        break;
    case 3:
        r = 1.0;
        g = 1.0 - s;
        b = 0.0;
        break;
    }

    return {1.0f * r, 1.0f * g, 1.0f * b};
}

void transformGridToVerticies()
{
    vertices.clear();
    indices.clear();

    std::array<glm::vec3, 8> cubeVertices = {
        glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f),
        glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, -0.5f),
        glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f),
        glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f)};

    std::array<unsigned int, 36> cubeIndices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        4, 0, 3, 3, 7, 4,
        1, 5, 6, 6, 2, 1,
        3, 2, 6, 6, 7, 3,
        4, 5, 1, 1, 0, 4};

    for (uint16_t x = 0; x < mac.size_x; ++x)
    {
        for (uint16_t y = 0; y < mac.size_y; ++y)
        {
            glm::vec3 cubePos = glm::vec3(x, y, 0) * mac.cell_size;
            for (const auto &vert : cubeVertices)
            {
                Vertex vertex;
                vertex.position = cubePos + vert * mac.cell_size;

                std::array<float, 3> color = getSciColor(mac.cells[x][y].p, -100.0f, 100.0f);
                if (mac.cells[x][y].s == 0.0f)
                {
                    color = {0.1f, 0.1f, 0.1f};
                }
                vertex.color = glm::vec3(color[0], color[1], color[2]);

                vertex.normal = glm::normalize(vert);
                vertices.push_back(vertex);
            }

            unsigned int offset = vertices.size() - cubeVertices.size();
            for (const auto &idx : cubeIndices)
            {
                indices.push_back(offset + idx);
            }
        }
    }
}

void render(VAO &vao)
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = glm::lookAt(camera.position, camera.position + camera.direction, camera.up);
    glm::mat4 projection = glm::perspective(glm::radians(camera.fov),
                                            (float)config.window_width / (float)config.window_height,
                                            camera.near_plane,
                                            camera.far_plane);
    glm::mat4 cameraMatrix = projection * view;

    glUseProgram(shaderProgram);

    glUniform3f(glGetUniformLocation(shaderProgram, "scale"), 1.0f, 1.0f, 1.0f);
    glUniform3f(glGetUniformLocation(shaderProgram, "rotation"), 0.0f, 0.0f, 0.0f);
    glUniform3f(glGetUniformLocation(shaderProgram, "position"), 0.0f, 0.0f, 0.0f);

    glUniform4f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f, 1.0f);
    glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 0.0f, 10.0f, 0.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "camMatrix"), 1, GL_FALSE, glm::value_ptr(cameraMatrix));

    vao.bind();
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

void cleanup()
{
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
}

int main()
{
    if (!initializeWindow())
        return -1;

    shaderProgram = createShaderProgram();
    VAO vao;
    setupBuffers(vao);

    while (!glfwWindowShouldClose(window))
    {
        processInput();
        render(vao);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanup();
    return 0;
}

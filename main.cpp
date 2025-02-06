#include <cstdint>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "inc/buffers/vao.h"
#include "inc/buffers/vbo.h"
#include "inc/buffers/ebo.h"

struct Config
{
    float fov = 45.0f;
    float near_plane = 0.1f;
    float far_plane = 100.0f;
    uint32_t window_width = 1200;
    uint32_t window_height = 900;
    glm::vec3 camera_position = glm::vec3(15.71f, 8.86f, 38.01f);
    glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 camera_direction = glm::vec3(0.0f, 0.0f, -1.0f);
};

Config config;
GLFWwindow *window;
GLuint shaderProgram;

std::vector<Vertex> vertices = {
    {{-1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    {{-1.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
    {{1.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
    {{1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}};
std::vector<GLuint> indices = {0, 1, 2, 0, 2, 3};

std::string loadShaderFile(const char *filename);
bool initializeWindow();
GLuint createShaderProgram();
void setupBuffers(VAO &vao, VBO &vbo, EBO &ebo);
void processInput();
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
}

void render(VAO &vao)
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = glm::lookAt(config.camera_position, config.camera_position + config.camera_direction, config.camera_up);
    glm::mat4 projection = glm::perspective(glm::radians(config.fov),
                                            (float)config.window_width / (float)config.window_height,
                                            config.near_plane,
                                            config.far_plane);
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

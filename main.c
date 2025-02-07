#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <array>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "inc/camera.h"

#include "inc/mac.h"

#include "inc/buffers/vao.h"
#include "inc/buffers/vbo.h"
#include "inc/buffers/ebo.h"

#include "inc/data.h"


#define VERTICIES_SIZE 200000
#define INDICIES_SIZE 200000

struct CoreConfig
{
    uint32_t window_width = 1200;
    uint32_t window_height = 900;
};

CoreConfig config;
Camera camera;
MacGrid mac;

GLFWwindow *window;
GLuint shaderProgram;

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    config.window_width = width;
    config.window_height = height;
    glViewport(0, 0, width, height);
}

char *loadShaderFile(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Error loading shader: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(fileSize + 1);
    if (!buffer)
    {
        fprintf(stderr, "Memory allocation failed for shader file\n");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, fileSize, file);
    buffer[fileSize] = '\0'; 

    fclose(file);
    return buffer;
}

bool initializeWindow()
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(config.window_width, config.window_height, "FLUID SIMULATION", nullptr, nullptr);
    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (!gladLoadGL())
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    return true;
}

GLuint createShaderProgram()
{
    const char *vertexShaderSource = loadShaderFile("./resources/shaders/default.vert");
    const char *fragmentShaderSource = loadShaderFile("./resources/shaders/default.frag");

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

    free((void *)vertexShaderSource);
    free((void *)fragmentShaderSource);

    return program;
}

void processInput()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    processCameraInput(window, camera);
}

void setupBuffers(VAO &vao, VBO &vbo, EBO &ebo)
{
    VAO_init(&vao);
    VBO_init(&vbo, VERTICIES_SIZE);
    EBO_init(&ebo, INDICIES_SIZE);

    VAO_bind(&vao);
    VBO_bind(&vbo);
    EBO_bind(&ebo);

    VAO_linkAttrib(0, 3, GL_FLOAT, sizeof(Vertex), (void *)0);
    VAO_linkAttrib(1, 3, GL_FLOAT, sizeof(Vertex), (void *)(3 * sizeof(float)));
    VAO_linkAttrib(2, 3, GL_FLOAT, sizeof(Vertex), (void *)(6 * sizeof(float)));
    VAO_linkAttrib(3, 2, GL_FLOAT, sizeof(Vertex), (void *)(9 * sizeof(float)));

    VAO_unbind();
    VBO_unbind();
    EBO_unbind();
}

void render(Camera &camera, VAO &vao, VBO &vbo, EBO &ebo, Vertex *vertices, GLuint *indices)
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

    VBO_bind(&vbo);
    VBO_update(&vbo, vertices, VERTICIES_SIZE);

    EBO_bind(&ebo);
    EBO_update(&ebo, indices, INDICIES_SIZE);

    VAO_bind(&vao);
    glDrawElements(GL_TRIANGLES, INDICIES_SIZE, GL_UNSIGNED_INT, 0);

    VAO_unbind();
    VBO_unbind();
    EBO_unbind();

    glfwSwapBuffers(window);
    glfwPollEvents();
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

    Vertex *vertices = (Vertex *)calloc(VERTICIES_SIZE, sizeof(Vertex));
    GLuint *indices = (GLuint *)calloc(INDICIES_SIZE, sizeof(GLuint));

    shaderProgram = createShaderProgram();

    VAO vao;
    VBO vbo;
    EBO ebo;

    setupBuffers(vao, vbo, ebo);

    MAC_init(mac, 50, 50, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        processInput();
        MAC_transformGridToVerticies(mac, vertices, indices);
        render(camera, vao, vbo, ebo, vertices, indices);
    }

    cleanup();
    return 0;
}

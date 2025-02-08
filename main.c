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
#include "inc/obstacle.h"

#include "inc/buffers/shader.h"
#include "inc/buffers/vao.h"
#include "inc/buffers/vbo.h"
#include "inc/buffers/ebo.h"

#include "inc/data.h"

#define VERTICIES_SIZE 200000
#define INDICIES_SIZE 200000

typedef struct CoreConfig
{
    uint32_t window_width;
    uint32_t window_height;
} CoreConfig_t;

CoreConfig_t config;

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    config.window_width = width;
    config.window_height = height;
    glViewport(0, 0, width, height);
}

GLFWwindow *initializeWindow()
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return NULL;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(config.window_width, config.window_height, "FLUID SIMULATION", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (!gladLoadGL())
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        glfwTerminate();
        return NULL;
    }

    glEnable(GL_DEPTH_TEST);
    return window;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

void setupBuffers(VAO_t &vao, VBO_t &vbo, EBO_t &ebo)
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
void setupObstacleBuffers(VAO_t &vao, VBO_t &vbo, EBO_t &ebo)
{
    VAO_init(&vao);
    VBO_init(&vbo, VERTICIES_SIZE);
    EBO_init(&ebo, INDICIES_SIZE);

    VAO_bind(&vao);
    VBO_bind(&vbo);
    EBO_bind(&ebo);

    VAO_linkAttrib(0, 3, GL_FLOAT, sizeof(Vertex), (void *)0);                   // Pozycja
    VAO_linkAttrib(1, 3, GL_FLOAT, sizeof(Vertex), (void *)(3 * sizeof(float))); // Normale
    VAO_linkAttrib(2, 3, GL_FLOAT, sizeof(Vertex), (void *)(6 * sizeof(float))); // Kolor
    VAO_linkAttrib(3, 2, GL_FLOAT, sizeof(Vertex), (void *)(9 * sizeof(float))); // Współrzędne UV

    VAO_unbind();
    VBO_unbind();
    EBO_unbind();
}

void render(GLFWwindow *window, Camera_t &camera, Shader_t &shaderProgram, VAO_t &vao, VBO_t &vbo, EBO_t &ebo, Vertex_t *vertices, GLuint *indices)
{
    Shader_use(&shaderProgram);
    Shader_setVector3f(&shaderProgram, "scale", 1.0f, 1.0f, 1.0f);
    Shader_setVector3f(&shaderProgram, "rotation", 0.0f, 0.0f, 0.0f);
    Shader_setVector3f(&shaderProgram, "position", 0.0f, 0.0f, 0.0f);

    Shader_setVector4f(&shaderProgram, "lightColor", 1.0f, 1.0f, 1.0f, 1.0f);
    Shader_setVector3f(&shaderProgram, "lightPos", 0.0f, 10.0f, 0.0f);
    Shader_setMatrix4f(&shaderProgram, "camMatrix", glm::value_ptr(camera.cam_mat));

    VBO_bind(&vbo);
    VBO_update(&vbo, vertices, VERTICIES_SIZE);

    EBO_bind(&ebo);
    EBO_update(&ebo, indices, INDICIES_SIZE);

    VAO_bind(&vao);
    glDrawElements(GL_TRIANGLES, INDICIES_SIZE, GL_UNSIGNED_INT, 0);

    VAO_unbind();
    VBO_unbind();
    EBO_unbind();
}

int main(int argc, char **argv)
{
    config.window_width = 800;
    config.window_height = 600;

    if (argc >= 3)
    {
        char *endptr_w, *endptr_h;
        uint32_t w = (uint32_t)strtoul(argv[1], &endptr_w, 10);
        uint32_t h = (uint32_t)strtoul(argv[2], &endptr_h, 10);

        if (*endptr_w == '\0' && *endptr_h == '\0' && w > 0 && h > 0)
        {
            config.window_width = w;
            config.window_height = h;
        }
        else
        {
            fprintf(stderr, "Invalid window size arguments. Using default 800x600.\n");
        }
    }
    else
    {
        printf("Usage: %s <width> <height>\nUsing default: 800x600\n", argv[0]);
    }

    GLFWwindow *window = initializeWindow();

    if (!window)
        return -1;

    Vertex_t *vertices = (Vertex *)calloc(VERTICIES_SIZE, sizeof(Vertex));
    GLuint *indices = (GLuint *)calloc(INDICIES_SIZE, sizeof(GLuint));

    Vertex_t *obstacleVertices = (Vertex *)calloc(VERTICIES_SIZE, sizeof(Vertex));
    GLuint *obstacleIndices = (GLuint *)calloc(INDICIES_SIZE, sizeof(GLuint));

    Shader_t fluidShader;
    Shader_init(&fluidShader, "shaders/fluid.vert", "shaders/fluid.frag");

    Shader_t obstacleShader;
    Shader_init(&obstacleShader, "shaders/obstacle.vert", "shaders/obstacle.frag");

    VAO_t fluidVao, obstacleVao;
    VBO_t fluidVbo, obstacleVbo;
    EBO_t fluidEbo, obstacleEbo;

    setupBuffers(fluidVao, fluidVbo, fluidEbo);
    setupObstacleBuffers(obstacleVao, obstacleVbo, obstacleEbo);

    Camera_t camera;
    Camera_init(&camera, window, 45.0f, 0.1f, 100.0f);

    MacGrid_t mac;
    MAC_init(&mac, 50, 50, 1.0f);

    Obstacle_t obstacle;
    Obstacle_init(&obstacle, glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 0.3f);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        processInput(window);
        Camera_processInput(&camera, window);
        Obstacle_processInput(&obstacle, window);

        MAC_handleObstacle(&mac, obstacle.position, obstacle.radius);
        MAC_update(&mac, 0.1f);

        MAC_transformGridToVerticies(&mac, vertices, indices);
        Obstacle_transformToVertices(&obstacle, obstacleVertices, obstacleIndices);

        render(window, camera, fluidShader, fluidVao, fluidVbo, fluidEbo, vertices, indices);
        render(window, camera, obstacleShader, obstacleVao, obstacleVbo, obstacleEbo, obstacleVertices, obstacleIndices);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Obstacle_destroy(&obstacle);
    MAC_destroy(&mac);
    VAO_destroy(&fluidVao);
    VBO_destroy(&fluidVbo);
    EBO_destroy(&fluidEbo);
    VAO_destroy(&obstacleVao);
    VBO_destroy(&obstacleVbo);
    EBO_destroy(&obstacleEbo);
    Shader_destroy(&fluidShader);
    Shader_destroy(&obstacleShader);
    Camera_destroy(&camera);
    free(vertices);
    free(indices);
    free(obstacleVertices);
    free(obstacleIndices);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

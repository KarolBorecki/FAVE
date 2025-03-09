#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <array>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "definitions.h"

#include "inc/types.h"
#include "inc/camera.h"

#include "inc/flip.h"
#include "inc/obstacle.h"

#include "inc/buffers/shader.h"
#include "inc/buffers/vao.h"
#include "inc/buffers/vbo.h"
#include "inc/buffers/ebo.h"

#include "inc/data.h"

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

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        glfwTerminate();
        return NULL;
    }

    glEnable(GL_DEPTH_TEST);

    // FOR DEBUGGING
    // glEnable(GL_DEBUG_OUTPUT);
    // glDebugMessageCallback(GLDebugMessageCallback, 0);

    return window;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

void setupBuffers(VAO_t &vao, VBO_t &vbo, EBO_t &ebo, long verticies_size, long indicies_size)
{
    VAO_init(&vao);
    VBO_init(&vbo, verticies_size);
    EBO_init(&ebo, indicies_size);

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

void render(GLFWwindow *window, Camera_t &camera, Shader_t &shaderProgram, VAO_t &vao, VBO_t &vbo, EBO_t &ebo, Vertex_t *fluid_vertices, GLuint *fluid_indices, int verticies_size, int indicies_size)
{
    if (verticies_size > MAX_VERTICIES || indicies_size > MAX_INDICIES)
    {
        fprintf(stderr, "Too many verticies or indicies to render\n");
        return;
    }

    Shader_use(&shaderProgram);
    Shader_setVector3f(&shaderProgram, "scale", 1.0f, 1.0f, 1.0f);
    Shader_setVector3f(&shaderProgram, "rotation", 0.0f, 0.0f, 0.0f);
    Shader_setVector3f(&shaderProgram, "position", 0.0f, 0.0f, 0.0f);

    Shader_setVector4f(&shaderProgram, "lightColor", 1.0f, 1.0f, 1.0f, 1.0f);
    Shader_setVector3f(&shaderProgram, "lightPos", 0.0f, 10.0f, 0.0f);
    Shader_setMatrix4f(&shaderProgram, "camMatrix", glm::value_ptr(camera.cam_mat));

    VBO_bind(&vbo);
    VBO_update(&vbo, fluid_vertices, verticies_size);

    EBO_bind(&ebo);
    EBO_update(&ebo, fluid_indices, indicies_size);

    VAO_bind(&vao);
    glDrawElements(GL_TRIANGLES, indicies_size, GL_UNSIGNED_INT, 0);

    VAO_unbind();
    VBO_unbind();
    EBO_unbind();
}

void parseArgument(const char *arg, const char *key, void *value, const char *type)
{
    size_t key_len = strlen(key);
    if (strncmp(arg, key, key_len) == 0 && arg[key_len] == '=')
    {
        if (strcmp(type, "float") == 0)
            *((float *)value) = strtof(arg + key_len + 1, NULL);
        else if (strcmp(type, "int") == 0)
            *((int *)value) = atoi(arg + key_len + 1);
    }
}

int main(int argc, char **argv)
{
    config.window_width = 1400;
    config.window_height = 1200;

    GLFWwindow *window = initializeWindow();

    if (!window)
        return -1;

    Vertex_t *fluid_vertices = (Vertex_t *)calloc(FLUID_VERTICIES_SIZE, sizeof(Vertex_t));
    GLuint *fluid_indices = (GLuint *)calloc(FLUID_INDICIES_SIZE, sizeof(GLuint));

    Vertex_t *obstacle_vertices = (Vertex_t *)calloc(OBSTACLE_VERTICIES_SIZE, sizeof(Vertex_t));
    GLuint *obstacle_indices = (GLuint *)calloc(OBSTACLE_INDICIES_SIZE, sizeof(GLuint));

    Vertex_t *marker_vertices = (Vertex_t *)calloc(PARTICLES_VERTICIES_SIZE, sizeof(Vertex_t));
    GLuint *marker_indices = (GLuint *)calloc(PARTICLES_INDICIES_SIZE, sizeof(GLuint));

    if (!fluid_vertices || !fluid_indices || !obstacle_vertices || !obstacle_indices || !marker_vertices || !marker_indices)
    {
        fprintf(stderr, "Failed to allocate memory for fluid_vertices and fluid_indices\n");
        return -1;
    }

    Shader_t fluidShader, obstacleShader, markerShader;
    Shader_init(&fluidShader, "shaders/default.vert", "shaders/default.frag");
    Shader_init(&obstacleShader, "shaders/obstacle.vert", "shaders/obstacle.frag");
    Shader_init(&markerShader, "shaders/default.vert", "shaders/default.frag");

    VAO_t fluidVao, obstacleVao, markerVao;
    VBO_t fluidVbo, obstacleVbo, markerVbo;
    EBO_t fluidEbo, obstacleEbo, markerEbo;

    setupBuffers(fluidVao, fluidVbo, fluidEbo, FLUID_VERTICIES_SIZE, FLUID_INDICIES_SIZE);
    setupBuffers(obstacleVao, obstacleVbo, obstacleEbo, OBSTACLE_VERTICIES_SIZE, OBSTACLE_INDICIES_SIZE);
    setupBuffers(markerVao, markerVbo, markerEbo, PARTICLES_VERTICIES_SIZE, PARTICLES_INDICIES_SIZE);

    float flip_ratio = 0.9f;
    float over_relaxation = 1.9f;
    int pressure_solver_steps = 100;
    int particles_push_apart_steps = 2;
    int show_markers = 1;
    int show_sci = 0;
    float gravity = -9.81f;
    float density = 1000.0f;
    float spacing = 0.05f;
    float size_x = 1.5f;
    float size_y = 1.5f;
    float size_z = 1.5f;
    float particle_radius = 0.02f;
    int max_particles = 5000;

    // float flip_ratio = 0.5f;
    // float over_relaxation = 1.2f;
    // int pressure_solver_steps = 200;
    // int particles_push_apart_steps = 2;
    // int show_markers = 1;
    // int show_sci = 0;
    // float gravity = -90.0f;
    // float density = 1000.0f;
    // float spacing = 1.0f;
    // float size_x = 15.0f;
    // float size_y = 15.0f;
    // float particle_radius = 0.1f;
    // int max_particles = 200;

    for (int i = 1; i < argc; i++)
    {
        parseArgument(argv[i], "flip_ratio", &flip_ratio, "float");
        parseArgument(argv[i], "over_relaxation", &over_relaxation, "float");
        parseArgument(argv[i], "pressure_solver_steps", &pressure_solver_steps, "int");
        parseArgument(argv[i], "particles_push_apart_steps", &particles_push_apart_steps, "int");
        parseArgument(argv[i], "show_markers", &show_markers, "int");
        parseArgument(argv[i], "show_sci", &show_sci, "int");
        parseArgument(argv[i], "gravity", &gravity, "float");
        parseArgument(argv[i], "density", &density, "float");
        parseArgument(argv[i], "spacing", &spacing, "float");
        parseArgument(argv[i], "size_x", &size_x, "float");
        parseArgument(argv[i], "size_y", &size_y, "float");
        parseArgument(argv[i], "particle_radius", &particle_radius, "float");
        parseArgument(argv[i], "max_particles", &max_particles, "int");
    }

    float camera_speed = 0.1f;

    float obstacle_speed = 0.05f;
    float obstacle_radius = 0.2f;
    float obstacle_push = 50.0f;

    Camera_t camera;
    // camera, window, postion, speed, fov, near, far
    Camera_init(&camera, window, glm::vec3(size_x / 2.0f, size_y / 2.0f, (size_x + 300.0f) * spacing), camera_speed, 45.0f, 0.1f, 1000.0f);

    FlipGrid_t mac;
    FLIP_init(&mac, density, size_x, size_y, size_z, spacing, particle_radius, max_particles);

    Obstacle_t obstacle;
    // obstacle, x, y, z, radius, speed
    Obstacle_init(&obstacle, size_x / 2.0f, size_y + obstacle_radius * 2, obstacle_radius, obstacle_radius, obstacle_speed, obstacle_push);

    clock_t previousTime = clock();
    float dt = 1.0f / 60.0f; // TODO it should be calculated based on the time between frames or more sophisticated way
    while (!glfwWindowShouldClose(window))
    {
        clock_t currentTime = clock();
        dt = (float)(currentTime - previousTime) / CLOCKS_PER_SEC;
        // dt = 1.0f / 60.0f; 
        previousTime = currentTime;

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        processInput(window);

        Camera_processInput(&camera, window);
        Obstacle_processInput(&obstacle, window);

        FLIP_integrateParticles(&mac, dt, gravity);
        FLIP_pushParticlesApart(&mac, particles_push_apart_steps, dt);
        FLIP_handleObstacle(&mac, &obstacle, dt);
        FLIP_transferVelocities(&mac, 1, flip_ratio);
        FLIP_updateParticleDensity(&mac);
        FLIP_solveIncompressibility(&mac, pressure_solver_steps, dt, over_relaxation);
        FLIP_transferVelocities(&mac, 0, flip_ratio);

        Pair_t mac_grid_render_sizes = FLIP_transformGridToVerticies(&mac, fluid_vertices, fluid_indices, show_sci);
        render(window, camera, fluidShader, fluidVao, fluidVbo, fluidEbo, fluid_vertices, fluid_indices, mac_grid_render_sizes.first, mac_grid_render_sizes.second);

        if (show_markers)
        {
            Pair_t mac_markers_render_sizes = FLIP_transformMarkersToVertices(&mac, marker_vertices, marker_indices);
            render(window, camera, markerShader, markerVao, markerVbo, markerEbo, marker_vertices, marker_indices, mac_markers_render_sizes.first, mac_markers_render_sizes.second);
        }

        Pair_t obstacle_render_sizes = Obstacle_transformToVertices(&obstacle, obstacle_vertices, obstacle_indices);
        render(window, camera, obstacleShader, obstacleVao, obstacleVbo, obstacleEbo, obstacle_vertices, obstacle_indices, obstacle_render_sizes.first, obstacle_render_sizes.second);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Obstacle_destroy(&obstacle);
    FLIP_destroy(&mac);
    VAO_destroy(&fluidVao);
    VBO_destroy(&fluidVbo);
    EBO_destroy(&fluidEbo);
    VAO_destroy(&obstacleVao);
    VBO_destroy(&obstacleVbo);
    EBO_destroy(&obstacleEbo);
    Shader_destroy(&fluidShader);
    Shader_destroy(&obstacleShader);
    Camera_destroy(&camera);
    free(fluid_vertices);
    free(fluid_indices);
    free(obstacle_vertices);
    free(obstacle_indices);

    free(marker_vertices);
    free(marker_indices);
    Shader_destroy(&markerShader);
    VAO_destroy(&markerVao);
    VBO_destroy(&markerVbo);
    EBO_destroy(&markerEbo);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
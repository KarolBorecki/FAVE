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

void render(GLFWwindow *window, Camera_t &camera, Shader_t &shaderProgram, VAO_t &vao, VBO_t &vbo, EBO_t &ebo, Vertex_t *vertices, GLuint *indices, uint16_t verticies_size, uint16_t indicies_size)
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
    if (verticies_size > VERTICIES_SIZE || indicies_size > INDICIES_SIZE)
    {
        fprintf(stderr, "Too many verticies or indicies to render\n");
        return;
    }
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

    float flip_ratio = 0.9f;
    float over_relaxation = 1.9f;
    int pressure_solver_steps = 50;
    int particles_push_apart_steps = 3;
    int show_markers = 1;
    int show_sci = 0;
    float gravity = -9.81f;
    float density = 1000.0f;
    float spacing = 0.4f;
    float width = 20.0f;
    float height = 20.0f;
    float particle_radius = 0.009f;
    int max_particles = 500;

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
        parseArgument(argv[i], "width", &width, "float");
        parseArgument(argv[i], "height", &height, "float");
        parseArgument(argv[i], "particle_radius", &particle_radius, "float");
        parseArgument(argv[i], "max_particles", &max_particles, "int");
    }

    printf("Simulation Parameters:\n");
    printf("flip_ratio = %.2f\n", flip_ratio);
    printf("over_relaxation = %.2f\n", over_relaxation);
    printf("pressure_solver_steps = %d\n", pressure_solver_steps);
    printf("particles_push_apart_steps = %d\n", particles_push_apart_steps);
    printf("show_markers = %d\n", show_markers);
    printf("show_sci = %d\n", show_sci);
    printf("gravity = %.2f\n", gravity);
    printf("density = %.2f\n", density);
    printf("spacing = %.2f\n", spacing);
    printf("width = %.2f\n", width);
    printf("height = %.2f\n", height);
    printf("particle_radius = %.3f\n", particle_radius);
    printf("max_particles = %d\n", max_particles);

    GLFWwindow *window = initializeWindow();

    if (!window)
        return -1;

    Vertex_t *vertices = (Vertex_t *)calloc(VERTICIES_SIZE, sizeof(Vertex_t));
    GLuint *indices = (GLuint *)calloc(INDICIES_SIZE, sizeof(GLuint));

    Vertex_t *obstacleVertices = (Vertex_t *)calloc(VERTICIES_SIZE, sizeof(Vertex_t));
    GLuint *obstacleIndices = (GLuint *)calloc(INDICIES_SIZE, sizeof(GLuint));

    Vertex_t *markerVertices = (Vertex_t *)calloc(VERTICIES_SIZE, sizeof(Vertex_t));
    GLuint *markerIndices = (GLuint *)calloc(INDICIES_SIZE, sizeof(GLuint));

    if (!vertices || !indices || !obstacleVertices || !obstacleIndices || !markerVertices || !markerIndices)
    {
        fprintf(stderr, "Failed to allocate memory for vertices and indices\n");
        return -1;
    }

    Shader_t fluidShader, obstacleShader, markerShader;
    Shader_init(&fluidShader, "shaders/default.vert", "shaders/default.frag");
    Shader_init(&obstacleShader, "shaders/obstacle.vert", "shaders/obstacle.frag");
    Shader_init(&markerShader, "shaders/default.vert", "shaders/default.frag");

    VAO_t fluidVao, obstacleVao, markerVao;
    VBO_t fluidVbo, obstacleVbo, markerVbo;
    EBO_t fluidEbo, obstacleEbo, markerEbo;

    setupBuffers(fluidVao, fluidVbo, fluidEbo);
    setupObstacleBuffers(obstacleVao, obstacleVbo, obstacleEbo);
    setupBuffers(markerVao, markerVbo, markerEbo);

    Camera_t camera;
    // camera, window, postion, speed, fov, near, far
    Camera_init(&camera, window, glm::vec3(width / 2.0f, height / 2.0f, (width + 20.0f) * spacing), 5.0f, 45.0f, 0.1f, 1000.0f);

    FlipGrid_t flip;
    FLIP_init(&flip, density, width, height, spacing, particle_radius, max_particles);

    Obstacle_t obstacle;
    // obstacle, postion, radius, speed
    Obstacle_init(&obstacle, glm::vec3(1.0f, 40.0f, 0.0f), 5.1f, 3.1f);

    float dt = 1.0f / 60.0f; // TODO it should be calculated based on the time between frames or more sophisticated way
    int frames = -1;
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        processInput(window);

        Camera_processInput(&camera, window);
        Obstacle_processInput(&obstacle, window);

        if (frames > 0 || frames <= -1)
        {
            FLIP_integrateParticles(&flip, dt, gravity);
            FLIP_pushParticlesApart(&flip, particles_push_apart_steps, dt);
            FLIP_handleObstacle(&flip, &obstacle, dt);
            FLIP_transferVelocities(&flip, 1, flip_ratio);
            FLIP_updateParticleDensity(&flip);
            FLIP_solveIncompressibility(&flip, pressure_solver_steps, dt, over_relaxation);
            FLIP_transferVelocities(&flip, 0, flip_ratio);
            frames--;
        }
        
        Pair_t flip_grid_render_sizes = FLIP_transformGridToVerticies(&flip, vertices, indices, show_sci);
        render(window, camera, fluidShader, fluidVao, fluidVbo, fluidEbo, vertices, indices, flip_grid_render_sizes.first, flip_grid_render_sizes.second);

        if (show_markers)
        {
            Pair_t flip_markers_render_sizes = FLIP_transformMarkersToVertices(&flip, markerVertices, markerIndices);
            render(window, camera, markerShader, markerVao, markerVbo, markerEbo, markerVertices, markerIndices, flip_markers_render_sizes.first, flip_markers_render_sizes.second);
        }
    
    Pair_t obstacle_render_sizes = Obstacle_transformToVertices(&obstacle, obstacleVertices, obstacleIndices);
    render(window, camera, obstacleShader, obstacleVao, obstacleVbo, obstacleEbo, obstacleVertices, obstacleIndices, obstacle_render_sizes.first, obstacle_render_sizes.second);

    glfwSwapBuffers(window);
    glfwPollEvents();
}

Obstacle_destroy(&obstacle);
FLIP_destroy(&flip);
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

free(markerVertices);
free(markerIndices);
Shader_destroy(&markerShader);
VAO_destroy(&markerVao);
VBO_destroy(&markerVbo);
EBO_destroy(&markerEbo);
glfwDestroyWindow(window);
glfwTerminate();
return 0;
}

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

#include "inc/mac.h"
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

int main(int argc, char **argv)
{
    printf("./FAVE width=2024 height=1240 flip_ratio=0.8 over_relaxation=1.63 pressure_solver_steps=150 particles_push_apart_steps=3 show_markes=1 show_sci=0 gravity=0 density=1000.0 cell_size=3.0 marker_size=0.1 marker_num=100\n\n");
    config.window_width = 1400;
    config.window_height = 1200;

    float flip_ratio = 0.9f;
    float over_relaxation = 1.90f;
    int pressure_solver_steps = 50;
    int particles_push_apart_steps = 2;
    int show_markes = 1;
    int show_sci = 0;
    float gravity = -9.81f;
    float density = 1000.0f;
    float spacing = 0.03f;
    float width = 2.017808219178082f;
    float height = 3.0f;
    // float particle_radius = 0.009f;
    float particle_radius = 0.009f;
    int max_particles = 1924;

    // float gravity = -9.81f;
    // float density = 1000.0f;
    // float cell_size = 0.3f;
    // float size_x = 20.017808219178082f;
    // float size_y = 30.0f;
    // int marker_num = 9176;
    // float marker_size = 0.9f;

    // for (int i = 1; i < argc; i++)
    // {
    //     if (strncmp(argv[i], "width=", 6) == 0)
    //     {
    //         config.window_width = atoi(argv[i] + 6);
    //     }
    //     else if (strncmp(argv[i], "height=", 7) == 0)
    //     {
    //         config.window_height = atoi(argv[i] + 7);
    //     }
    //     else if (strncmp(argv[i], "flip_ratio=", 11) == 0)
    //     {
    //         flip_ratio = atof(argv[i] + 11);
    //     }
    //     else if (strncmp(argv[i], "over_relaxation=", 16) == 0)
    //     {
    //         over_relaxation = atof(argv[i] + 16);
    //     }
    //     else if (strncmp(argv[i], "pressure_solver_steps=", 22) == 0)
    //     {
    //         pressure_solver_steps = atoi(argv[i] + 22);
    //     }
    //     else if (strncmp(argv[i], "particles_push_apart_steps=", 27) == 0)
    //     {
    //         particles_push_apart_steps = atoi(argv[i] + 27);
    //     }
    //     else if (strncmp(argv[i], "show_markes=", 12) == 0)
    //     {
    //         show_markes = atoi(argv[i] + 12);
    //     }
    //     else if (strncmp(argv[i], "show_sci=", 9) == 0)
    //     {
    //         show_sci = atoi(argv[i] + 9);
    //     }
    //     else if (strncmp(argv[i], "gravity=", 8) == 0)
    //     {
    //         gravity = atof(argv[i] + 8);
    //     }
    //     else if (strncmp(argv[i], "density=", 8) == 0)
    //     {
    //         density = atof(argv[i] + 8);
    //     }
    //     else if (strncmp(argv[i], "cell_size=", 10) == 0)
    //     {
    //         cell_size = atof(argv[i] + 10);
    //     }
    //     else if (strncmp(argv[i], "size_x=", 7) == 0)
    //     {
    //         size_x = atoi(argv[i] + 7);
    //     }
    //     else if (strncmp(argv[i], "size_y=", 7) == 0)
    //     {
    //         size_y = atoi(argv[i] + 7);
    //     }
    //     else if (strncmp(argv[i], "marker_num=", 11) == 0)
    //     {
    //         marker_num = atoi(argv[i] + 11);
    //     }
    //     else if (strncmp(argv[i], "marker_size=", 12) == 0)
    //     {
    //         marker_size = atof(argv[i] + 12);
    //     }
    // }

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
    Camera_init(&camera, window, glm::vec3(1.0f, 1.5f, 4.0f), 45.0f, 0.1f, 1000.0f);

    MacGrid_t mac;
    MAC_init(&mac, density, width, height, spacing, particle_radius, max_particles);

    Obstacle_t obstacle;
    Obstacle_init(&obstacle, glm::vec3(2.0f, 10.0f, 0.0f), 1.0f, 1.0f);

    //float dt = 1.0f / 60.0f; // TODO it should be calculated based on the time between frames or more sophisticated way
    float dt = 0.1666666666f;
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        processInput(window);

        Camera_processInput(&camera, window);
        Obstacle_processInput(&obstacle, window);

        MAC_integrateParticles(&mac, dt, gravity);
        MAC_pushParticlesApart(&mac, particles_push_apart_steps, dt);
        MAC_handleObstacle(&mac, &obstacle, dt);
        MAC_transferVelocities(&mac, 1, flip_ratio);
        MAC_updateParticleDensity(&mac);
        MAC_solveIncompressibility(&mac, pressure_solver_steps, dt, over_relaxation);
        MAC_transferVelocities(&mac, 0, flip_ratio);

        Pair_t mac_grid_render_sizes = MAC_transformGridToVerticies(&mac, vertices, indices, show_sci);
        render(window, camera, fluidShader, fluidVao, fluidVbo, fluidEbo, vertices, indices, mac_grid_render_sizes.first, mac_grid_render_sizes.second);

        if (show_markes)
        {
            printf("showing markers\n");
            Pair_t mac_markers_render_sizes = MAC_transformMarkersToVertices(&mac, markerVertices, markerIndices);
            render(window, camera, markerShader, markerVao, markerVbo, markerEbo, markerVertices, markerIndices, mac_markers_render_sizes.first, mac_markers_render_sizes.second);
        }

        Pair_t obstacle_render_sizes = Obstacle_transformToVertices(&obstacle, obstacleVertices, obstacleIndices);
        render(window, camera, obstacleShader, obstacleVao, obstacleVbo, obstacleEbo, obstacleVertices, obstacleIndices, obstacle_render_sizes.first, obstacle_render_sizes.second);

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

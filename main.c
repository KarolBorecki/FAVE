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
#include "inc/flip_2D.h"
#include "inc/obstacle.h"

#include "inc/buffers/shader.h"
#include "inc/buffers/vao.h"
#include "inc/buffers/vbo.h"
#include "inc/buffers/ebo.h"

#include "inc/data.h"

CoreConfig_t config;

void checkGLError(const char *message)
{
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        fprintf(stderr, "[OpenGL Error] %s - Kod błędu: %d\n", message, error);
    }
}

void load_config(const char *filename, CoreConfig_t &core_config)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Nie można otworzyć pliku konfiguracyjnego");
        exit(EXIT_FAILURE);
    }

    char key[64];
    float value;

    while (fscanf(file, "%63s = %f", key, &value) == 2)
    {
        if (strcmp(key, "flip_ratio") == 0)
            core_config.flip_ratio = value;
        else if (strcmp(key, "over_relaxation") == 0)
            core_config.over_relaxation = value;
        else if (strcmp(key, "pressure_solver_steps") == 0)
            core_config.pressure_solver_steps = (int)value;
        else if (strcmp(key, "particles_push_apart_steps") == 0)
            core_config.particles_push_apart_steps = (int)value;
        else if (strcmp(key, "show_markers") == 0)
            core_config.show_markers = (int)value;
        else if (strcmp(key, "show_cubes") == 0)
            core_config.show_cubes = (int)value;
        else if (strcmp(key, "show_sci") == 0)
            core_config.show_sci = (int)value;
        else if (strcmp(key, "show_air") == 0)
            core_config.show_air = (int)value;
        else if (strcmp(key, "show_solids") == 0)
            core_config.show_solids = (int)value;
        else if (strcmp(key, "marching_cubes") == 0)
            core_config.marching_cubes = (int)value;
        else if (strcmp(key, "use_2D") == 0)
            core_config.use_2D = (int)value;
        else if (strcmp(key, "gravity") == 0)
            core_config.gravity = value;
        else if (strcmp(key, "density") == 0)
            core_config.density = value;
        else if (strcmp(key, "spacing") == 0)
            core_config.spacing = value;
        else if (strcmp(key, "size_x") == 0)
            core_config.size_x = value;
        else if (strcmp(key, "size_y") == 0)
            core_config.size_y = value;
        else if (strcmp(key, "size_z") == 0)
            core_config.size_z = value;
        else if (strcmp(key, "particle_radius") == 0)
            core_config.particle_radius = value;
        else if (strcmp(key, "max_particles") == 0)
            core_config.max_particles = (int)value;
        else if (strcmp(key, "cam_x") == 0)
            core_config.cam_x = value;
        else if (strcmp(key, "cam_y") == 0)
            core_config.cam_y = value;
        else if (strcmp(key, "cam_z") == 0)
            core_config.cam_z = value;
        else if (strcmp(key, "cam_rot_x") == 0)
            core_config.cam_rot_x = value;
        else if (strcmp(key, "cam_rot_y") == 0)
            core_config.cam_rot_y = value;
        else if (strcmp(key, "cam_rot_z") == 0)
            core_config.cam_rot_z = value;
        else if (strcmp(key, "cam_speed") == 0)
            core_config.cam_speed = value;
        else if (strcmp(key, "obstacle_radius") == 0)
            core_config.obstacle_radius = value;
        else if (strcmp(key, "obstacle_push_coefficient") == 0)
            core_config.obstacle_push_coefficient = value;
        else if (strcmp(key, "obstacle_speed") == 0)
            core_config.obstacle_speed = value;
    }

    fclose(file);
}

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
        fprintf(stderr, "failed to initialize GLFW\n");
        return NULL;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(config.window_width, config.window_height, "FLUID SIMULATION", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "failed to create GLFW window\n");
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "failed to initialize GLAD\n");
        glfwTerminate();
        return NULL;
    }

    glShadeModel(GL_FLAT);
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
    if (verticies_size <= 0 || indicies_size <= 0 || verticies_size > MAX_VERTICIES || indicies_size > MAX_INDICIES)
    {
        fprintf(stderr, "incorrect verticies/indicies sizes: verticies=%d, indicies=%d\n", verticies_size, indicies_size);
        return;
    }

    Shader_use(&shaderProgram);
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

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        load_config("./configs/config", config);
    }
    else
    {
        load_config(argv[1], config);
    }
    printf("==============Config loaded successfully!==============\n\n");
    printf("gravity = %.2f\n", config.gravity);
    printf("density = %.2f\n\n", config.density);

    printf("show_markers = %d\n", config.show_markers);
    printf("show_cubes = %d\n", config.show_cubes);
    printf("show_sci = %d\n", config.show_sci);
    printf("marching_cubes = %d\n", config.marching_cubes);
    printf("use_2D = %d\n\n", config.use_2D);

    printf("pressure_solver_steps = %d\n", config.pressure_solver_steps);
    printf("particles_push_apart_steps = %d\n", config.particles_push_apart_steps);
    printf("flip_ratio = %.2f\n", config.flip_ratio);
    printf("over_relaxation = %.2f\n\n", config.over_relaxation);

    printf("spacing = %.2f\n", config.spacing);
    printf("size_x = %.2f size_y = %.2f size_z = %.2f\n", config.size_x, config.size_y, config.size_z);
    printf("particle_radius = %.2f\n", config.particle_radius);
    printf("max_particles = %d\n", config.max_particles);
    printf("cam_x = %.2f cam_y = %.2f cam_z = %.2f\n", config.cam_x, config.cam_y, config.cam_z);
    printf("cam_rot_x = %.2f cam_rot_y = %.2f cam_rot_z = %.2f\n", config.cam_rot_x, config.cam_rot_y, config.cam_rot_z);
    printf("cam_speed = %.2f\n", config.cam_speed);
    printf("obstacle_radius = %.2f\n", config.obstacle_radius);
    printf("obstacle_push_coefficient = %.2f\n", config.obstacle_push_coefficient);
    printf("obstacle_speed = %.2f\n\n", config.obstacle_speed);

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
        fprintf(stderr, "failed to allocate memory for vertices or indices\n");
        return -1;
    }

    Shader_t fluidShader, obstacleShader, markerShader;
    Shader_init(&fluidShader, "shaders/default.vert", "shaders/default.frag");
    Shader_init(&obstacleShader, "shaders/default.vert", "shaders/default.frag");
    Shader_init(&markerShader, "shaders/default.vert", "shaders/default.frag");

    VAO_t fluidVao, obstacleVao, markerVao;
    VBO_t fluidVbo, obstacleVbo, markerVbo;
    EBO_t fluidEbo, obstacleEbo, markerEbo;

    setupBuffers(fluidVao, fluidVbo, fluidEbo, FLUID_VERTICIES_SIZE, FLUID_INDICIES_SIZE);
    setupBuffers(obstacleVao, obstacleVbo, obstacleEbo, OBSTACLE_VERTICIES_SIZE, OBSTACLE_INDICIES_SIZE);
    setupBuffers(markerVao, markerVbo, markerEbo, PARTICLES_VERTICIES_SIZE, PARTICLES_INDICIES_SIZE);

    FlipGrid_t mac;
    if (!config.use_2D)
    {
        FLIP_init(&mac, config.density, config.size_x, config.size_y, config.size_z, config.spacing, config.particle_radius, config.max_particles);
    }
    else
    {
        FLIP2D_init(&mac, config.density, config.size_x, config.size_y, config.spacing, config.particle_radius, config.max_particles);
    }

    printf("==============FLIP grid initialized successfully!==============\n\n");
    printf("f_num_x = %d, f_num_y = %d, f_num_z = %d, f_num_cells = %d\n", mac.f_num_x, mac.f_num_y, mac.f_num_z, mac.f_num_cells);
    printf("p_num_x = %d, p_num_y = %d, p_num_z = %d, p_num_cells = %d\n", mac.p_num_x, mac.p_num_y, mac.p_num_z, mac.p_num_cells);
    printf("num_particles = %d, particle_radius = %.2f, p_inv_spacing = %.2f\n", mac.num_particles, mac.particle_radius, mac.p_inv_spacing);
    printf("particle_rest_density = %.2f\n", mac.particle_rest_density);
    printf("density = %.2f\n\n", mac.density);

    Camera_t camera;
    Camera_init(&camera, window, glm::vec3(config.cam_x, config.cam_y, config.cam_z), glm ::vec3(config.cam_rot_x, config.cam_rot_y, config.cam_rot_z), config.cam_speed, 45.0f, 0.1f, 1000.0f);

    Obstacle_t obstacle;
    Obstacle_init(&obstacle, config.size_x / 2.0f, config.size_y, config.size_z / 2.0f, config.obstacle_radius, config.obstacle_speed, config.obstacle_push_coefficient);

    clock_t previousTime = clock();
    float dt = 1.0f / 60.0f;
    while (!glfwWindowShouldClose(window))
    {
        clock_t currentTime = clock();
        dt = (float)(currentTime - previousTime) / CLOCKS_PER_SEC;
        previousTime = currentTime;

        glClearColor(0.823529411765f, 0.8f, 0.752941176471f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        processInput(window);

        Camera_processInput(&camera, window);

        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            Obstacle_integrate(&obstacle, config.gravity, 0.0f, dt);
        }
        Obstacle_processInput(&obstacle, window);

        if (!config.use_2D)
        {
            FLIP_integrateParticles(&mac, dt, config.gravity);
            FLIP_pushParticlesApart(&mac, config.particles_push_apart_steps, dt);
            FLIP_handleObstacle(&mac, &obstacle, dt);
            FLIP_transferVelocities(&mac, 1, config.flip_ratio);
            FLIP_updateParticleDensity(&mac);
            FLIP_solveIncompressibility(&mac, config.pressure_solver_steps, dt, config.over_relaxation);
            FLIP_transferVelocities(&mac, 0, config.flip_ratio);
        }
        else
        {
            FLIP2D_integrateParticles(&mac, dt, config.gravity);
            FLIP2D_pushParticlesApart(&mac, config.particles_push_apart_steps, dt);
            FLIP2D_handleObstacle(&mac, &obstacle, dt);
            FLIP2D_transferVelocities(&mac, 1, config.flip_ratio);
            FLIP2D_updateParticleDensity(&mac);
            FLIP2D_solveIncompressibility(&mac, config.pressure_solver_steps, dt, config.over_relaxation);
            FLIP2D_transferVelocities(&mac, 0, config.flip_ratio);
        }

        if (config.show_cubes)
        {
            Pair_t mac_grid_render_sizes;
            if (!config.marching_cubes)
            {
                mac_grid_render_sizes = FLIP_transformGridToVerticies(&mac, fluid_vertices, fluid_indices, config.show_sci, config.show_air, config.show_solids);
            }
            else
            {
                // if (!config.use_2D)
                    mac_grid_render_sizes = FLIP_transformGridToVerticiesMarchingCubes(&mac, fluid_vertices, fluid_indices, config.show_sci, config.show_air, config.show_solids);
                // else
                //     mac_grid_render_sizes = FLIP2D_transformGridToVerticiesMarchingSquares(&mac, fluid_vertices, fluid_indices, config.show_sci, config.show_air, config.show_solids);
            }
            render(window, camera, fluidShader, fluidVao, fluidVbo, fluidEbo, fluid_vertices, fluid_indices, mac_grid_render_sizes.first, mac_grid_render_sizes.second);
        }

        if (config.show_markers)
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
    VAO_destroy(&markerVao);
    VBO_destroy(&markerVbo);
    EBO_destroy(&markerEbo);
    Shader_destroy(&fluidShader);
    Shader_destroy(&obstacleShader);
    Shader_destroy(&markerShader);

    Camera_destroy(&camera);

    free(fluid_vertices);
    free(fluid_indices);
    free(obstacle_vertices);
    free(obstacle_indices);
    free(marker_vertices);
    free(marker_indices);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
#ifndef FAVE_OBSTACLE_H
#define FAVE_OBSTACLE_H

#include <cstdio>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "types.h"
#include "buffers/vbo.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define PI 3.14159265359f

    typedef struct Obstacle
    {
        glm::vec3 position;
        glm::vec3 last_position;
        float x;
        float y;
        float z;
        float last_x;
        float last_y;
        float last_z;
        float radius;
        float speed;
        float push_coefficient;
    } Obstacle_t;

    void Obstacle_init(Obstacle_t *obstacle, float x, float y, float z, float radius, float speed, float push_coefficient);
    float Obstacle_getXVelocity(Obstacle_t *obstacle, float dt);
    float Obstacle_getYVelocity(Obstacle_t *obstacle, float dt);
    float Obstacle_getZVelocity(Obstacle_t *obstacle, float dt);
    void Obstacle_integrate(Obstacle_t *obstacle, float gravity, float min_y, float dt);
    void Obstacle_processInput(Obstacle_t *obstacle, GLFWwindow *window);
    Pair_t Obstacle_transformToVertices(Obstacle_t *obstacle, Vertex_t *vertices, GLuint *indices);
    void Obstacle_destroy(Obstacle_t *obstacle);

#ifdef __cplusplus
}
#endif

#endif // FAVE_OBSTACLE_H

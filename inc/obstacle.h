#ifndef FAVE_OBSTACLE_H
#define FAVE_OBSTACLE_H

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
    float radius;
    float speed;
} Obstacle_t;

void Obstacle_init(Obstacle_t *obstacle, glm::vec3 position, float radius, float speed);
void Obstacle_processInput(Obstacle_t *obstacle, GLFWwindow *window);
Pair_t Obstacle_transformToVertices(Obstacle_t *obstacle, Vertex_t *vertices, GLuint *indices);
void Obstacle_destroy(Obstacle_t *obstacle);

#ifdef __cplusplus
}
#endif

#endif // FAVE_OBSTACLE_H

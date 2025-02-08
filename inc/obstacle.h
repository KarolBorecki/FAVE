#ifndef FAVE_OBSTACLE_H
#define FAVE_OBSTACLE_H

#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "buffers/vbo.h"

#define PI 3.14159265359f

typedef struct Obstacle
{
    glm::vec3 position;
    float radius;
    float speed;
} Obstacle_t;

void Obstacle_init(Obstacle_t *obstacle, glm::vec3 position, float radius, float speed);
void Obstacle_processInput(Obstacle_t *obstacle, GLFWwindow *window);
void Obstacle_transformToVertices(Obstacle_t *obstacle, Vertex_t *vertices, GLuint *indices);
void Obstacle_destroy(Obstacle_t *obstacle);
#endif // FAVE_OBSTACLE_H
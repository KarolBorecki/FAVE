#include "obstacle.h"

void Obstacle_init(Obstacle_t *obstacle, float x, float y, float z, float radius, float speed, float push_coefficient)
{
    obstacle->x = x;
    obstacle->y = y;
    obstacle->z = z;

    obstacle->last_x = x;
    obstacle->last_y = y;
    obstacle->last_z = z;

    obstacle->radius = radius;
    obstacle->speed = speed;
    obstacle->push_coefficient = push_coefficient;
}

float Obstacle_getXVelocity(Obstacle_t *obstacle, float dt)
{
    return (obstacle->x - obstacle->last_x) * obstacle->speed / dt;
}

float Obstacle_getYVelocity(Obstacle_t *obstacle, float dt)
{
    return (obstacle->y - obstacle->last_y) * obstacle->speed / dt;
}

float Obstacle_getZVelocity(Obstacle_t *obstacle, float dt)
{
    return (obstacle->z - obstacle->last_z) * obstacle->speed / dt;
}

void Obstacle_integrate(Obstacle_t *obstacle, float gravity, float min_y, float dt)
{
    if (obstacle->y > min_y)
    {
        obstacle->y += gravity * dt;
    }
    else
    {
        obstacle->y = min_y;
    }
}

void Obstacle_processInput(Obstacle_t *obstacle, GLFWwindow *window)
{
    obstacle->last_x = obstacle->x;
    obstacle->last_y = obstacle->y;
    obstacle->last_z = obstacle->z;

    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        obstacle->x -= obstacle->speed;

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        obstacle->x += obstacle->speed;

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        obstacle->y += obstacle->speed;

    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        obstacle->y -= obstacle->speed;

    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        obstacle->z -= obstacle->speed;

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        obstacle->z += obstacle->speed;

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        obstacle->x = 0.75f;
        obstacle->y = 0.2f;
        obstacle->z = 2.3f;
        printf("PRESS U TO MOVE\n");
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        obstacle->x = 2.0f;
        obstacle->y = 0.2f;
        obstacle->z = 0.65f;
        printf("PRESS J TO MOVE\n");
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        obstacle->x = 2.0f;
        obstacle->y = 0.2f;
        obstacle->z = 2.0f;
        printf("PRESS U + J TO MOVE\n");
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
    {
        obstacle->x = 0.75f;
        obstacle->y = -1.0f;
        obstacle->z = 0.75f;
        printf("PRESS I TO MOVE\n");
    }
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
    {
        obstacle->x = 2.0f;
        obstacle->y = 1.44f;
        obstacle->z = 0.65f;
        printf("PRESS J + K TO MOVE\n");
    }
}

Pair_t Obstacle_transformToVertices(Obstacle_t *obstacle, Vertex_t *vertices, GLuint *indices)
{
    const int sectorCount = 36;
    const int stackCount = 18;

    int vertexIndex = 0;
    int indexIndex = 0;

    float radius = obstacle->radius;
    float centerX = obstacle->x;
    float centerY = obstacle->y;
    float centerZ = obstacle->z;

    for (int i = 0; i <= stackCount; ++i)
    {
        float stackAngle = M_PI / 2 - (i * M_PI / stackCount);
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectorCount; ++j)
        {
            float sectorAngle = j * 2 * M_PI / sectorCount;

            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            float nx = x / radius;
            float ny = y / radius;
            float nz = z / radius;

            vertices[vertexIndex].position = (vec3s){{x + centerX, y + centerY, z + centerZ}};
            vertices[vertexIndex].normal = (vec3s){{nx, ny, nz}};
            vertices[vertexIndex].color = (vec3s){{0.925f, 0.306f, 0.125f}};

            vertexIndex++;
        }
    }

    for (int i = 0; i < stackCount; ++i)
    {
        int k1 = i * (sectorCount + 1);
        int k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                indices[indexIndex++] = k1;
                indices[indexIndex++] = k2;
                indices[indexIndex++] = k1 + 1;
            }

            if (i != (stackCount - 1))
            {
                indices[indexIndex++] = k1 + 1;
                indices[indexIndex++] = k2;
                indices[indexIndex++] = k2 + 1;
            }
        }
    }

    return (Pair_t){.first = vertexIndex, .second = indexIndex};
}

void Obstacle_destroy(Obstacle_t *obstacle)
{
}

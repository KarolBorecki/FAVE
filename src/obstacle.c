#include "obstacle.h"

void Obstacle_init(Obstacle_t *obstacle, float x, float y, float z, float radius, float speed, float push_coefficient)
{
    obstacle->position = (vec3s){{x, y, z}};
    obstacle->last_position = (vec3s){{x, y, z}};
    obstacle->radius = radius;
    obstacle->speed = speed;
    obstacle->push_coefficient = push_coefficient;
}

float Obstacle_getXVelocity(Obstacle_t *obstacle, float dt)
{
    return (obstacle->position.raw[0] - obstacle->last_position.raw[0]) * obstacle->speed / dt;
}

float Obstacle_getYVelocity(Obstacle_t *obstacle, float dt)
{
    return (obstacle->position.raw[1] - obstacle->last_position.raw[1]) * obstacle->speed / dt;
}

float Obstacle_getZVelocity(Obstacle_t *obstacle, float dt)
{
    return (obstacle->position.raw[2] - obstacle->last_position.raw[2]) * obstacle->speed / dt;
}

void Obstacle_integrate(Obstacle_t *obstacle, float gravity, float min_y, float dt)
{
    if (obstacle->position.raw[1] > min_y)
    {
        obstacle->position.raw[1] += gravity * dt;
    }
    else
    {
        obstacle->position.raw[1] = min_y;
    }
}

void Obstacle_processInput(Obstacle_t *obstacle, GLFWwindow *window)
{
    obstacle->last_position = obstacle->position;

    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        obstacle->position.raw[0] -= obstacle->speed;

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        obstacle->position.raw[0] += obstacle->speed;

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        obstacle->position.raw[1] += obstacle->speed;

    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        obstacle->position.raw[1] -= obstacle->speed;

    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        obstacle->position.raw[2] -= obstacle->speed;

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        obstacle->position.raw[2] += obstacle->speed;

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        obstacle->position = (vec3s){{0.75f, 0.2f, 2.3f}};
        printf("PRESS U TO MOVE\n");
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        obstacle->position = (vec3s){{2.0f, 0.2f, 0.65f}};
        printf("PRESS J TO MOVE\n");
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        obstacle->position = (vec3s){{2.0f, 0.2f, 2.0f}};
        printf("PRESS U + J TO MOVE\n");
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
    {
        obstacle->position = (vec3s){{0.75f, -1.0f, 0.75f}};
        printf("PRESS I TO MOVE\n");
    }
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
    {
        obstacle->position = (vec3s){{2.0f, 1.44f, 0.65f}};
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
    float centerX = obstacle->position.raw[0];
    float centerY = obstacle->position.raw[1];
    float centerZ = obstacle->position.raw[2];

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

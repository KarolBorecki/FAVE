#include "obstacle.h"

void Obstacle_init(Obstacle_t *obstacle, float x, float y, float z, float radius, float speed, float push_coefficient)
{
    obstacle->x = x;
    obstacle->last_x = x;
    obstacle->y = y;
    obstacle->last_y = y;
    obstacle->z = z;
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

void Obstacle_processInput(Obstacle_t *obstacle, GLFWwindow *window)
{
    // if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
    // {
    //     obstacle->position += glm::vec3(0.0f, 0.0f, -1.0f * obstacle->speed);
    //     obstacle->velocity += glm::vec3(0.0f, 0.0f, -1.0f * obstacle->speed);
    // }
    // if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    // {
    //     obstacle->position += glm::vec3(0.0f, 0.0f, 1.0f * obstacle->speed);
    //     obstacle->velocity += glm::vec3(0.0f, 0.0f, 1.0f * obstacle->speed);
    // }
    obstacle->last_x = obstacle->x;
    obstacle->last_y = obstacle->y;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
    {
        obstacle->x += -1.0f * obstacle->speed;
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        obstacle->x += 1.0f * obstacle->speed;
    }
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
    {
        obstacle->y += 1.0f * obstacle->speed;
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    {
        obstacle->y += -1.0f * obstacle->speed;
    }
}

Pair_t Obstacle_transformToVertices(Obstacle_t *obstacle, Vertex_t *vertices, GLuint *indices)
{
    const int sectorCount = 36; // Ilość podziałów w poziomie (południki)
    const int stackCount = 18;  // Ilość podziałów w pionie (równoleżniki)

    int vertexIndex = 0;
    int indexIndex = 0;

    float radius = obstacle->radius;
    float centerX = obstacle->x;
    float centerY = obstacle->y;
    float centerZ = obstacle->z;

    // **Generowanie wierzchołków sfery**
    for (int i = 0; i <= stackCount; ++i)
    {
        float stackAngle = PI / 2 - (i * PI / stackCount); // Od -PI/2 do PI/2
        float xy = radius * cosf(stackAngle);              // r * cos(u)
        float z = radius * sinf(stackAngle);               // r * sin(u)

        for (int j = 0; j <= sectorCount; ++j)
        {
            float sectorAngle = j * 2 * PI / sectorCount; // Od 0 do 2PI

            float x = xy * cosf(sectorAngle); // r * cos(u) * cos(v)
            float y = xy * sinf(sectorAngle); // r * cos(u) * sin(v)

            float nx = x / radius;
            float ny = y / radius;
            float nz = z / radius;

            // Tworzenie wierzchołka
            vertices[vertexIndex].position.x = x + centerX;
            vertices[vertexIndex].position.y = y + centerY;
            vertices[vertexIndex].position.z = z + centerZ;
            vertices[vertexIndex].normal.x = nx;
            vertices[vertexIndex].normal.y = ny;
            vertices[vertexIndex].normal.z = nz;
            vertices[vertexIndex].color.x = 1.0f; // Kolor czerwony
            vertices[vertexIndex].color.y = 0.0f;
            vertices[vertexIndex].color.z = 0.0f;

            vertexIndex++;
        }
    }

    // **Generowanie indeksów siatki**
    for (int i = 0; i < stackCount; ++i)
    {
        int k1 = i * (sectorCount + 1); // Wiersz bieżący
        int k2 = k1 + sectorCount + 1;  // Następny wiersz

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            if (i != 0) // Górny trójkąt
            {
                indices[indexIndex++] = k1;
                indices[indexIndex++] = k2;
                indices[indexIndex++] = k1 + 1;
            }

            if (i != (stackCount - 1)) // Dolny trójkąt
            {
                indices[indexIndex++] = k1 + 1;
                indices[indexIndex++] = k2;
                indices[indexIndex++] = k2 + 1;
            }
        }
    }

    return {.first = vertexIndex, .second = indexIndex};
}

void Obstacle_destroy(Obstacle_t *obstacle)
{
    // Nothing to do here
}
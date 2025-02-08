#include "obstacle.h"

void Obstacle_init(Obstacle_t *obstacle, glm::vec3 position, float radius, float speed)
{
    obstacle->position = position;
    obstacle->radius = radius;
    obstacle->speed = speed;
}

void Obstacle_processInput(Obstacle_t *obstacle, GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
    {
        obstacle->position += glm::vec3(0.0f, 0.0f, -1.0f * obstacle->speed);
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    {
        obstacle->position += glm::vec3(0.0f, 0.0f, 1.0f * obstacle->speed);
    }
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
    {
        obstacle->position += glm::vec3(-1.0f * obstacle->speed, 0.0f, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        obstacle->position += glm::vec3(1.0f * obstacle->speed, 0.0f, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
    {
        obstacle->position += glm::vec3(0.0f, 1.0f * obstacle->speed, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
    {
        obstacle->position += glm::vec3(0.0f, -1.0f * obstacle->speed, 0.0f);
    }
}

void Obstacle_transformToVertices(Obstacle_t *obstacle, Vertex_t *vertices, GLuint *indices)
{
    const int sectorCount = 36; // Ilość podziałów w poziomie (południki)
    const int stackCount = 18;  // Ilość podziałów w pionie (równoleżniki)

    int vertexIndex = 0;
    int indexIndex = 0;

    float radius = obstacle->radius;
    float centerX = obstacle->position.x;
    float centerY = obstacle->position.y;
    float centerZ = obstacle->position.z;

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
}

void Obstacle_destroy(Obstacle_t *obstacle)
{
    // Nothing to do here
}
#ifndef FAVE_CAMERA_H
#define FAVE_CAMERA_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef struct 
{
    glm::vec3 position = glm::vec3(15.71f, 8.86f, 38.01f);
    glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    float fov = 45.0f;
    float near_plane = 0.1f;
    float far_plane = 100.0f;
    float speed = 0.05f;
    float sensitivity = 100.0f;

    uint8_t first_input_click = false;
} Camera;

// Note: This function opropable needs dt
void processCameraInput(GLFWwindow *window, Camera &camera);

#endif // FAVE_CAMERA_H
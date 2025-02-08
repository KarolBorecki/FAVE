#ifndef FAVE_CAMERA_H
#define FAVE_CAMERA_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef struct Camera
{
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 up;

    float fov;
    float near_plane;
    float far_plane;
    float speed;
    float sensitivity;

    uint8_t first_input_click;

    glm::mat4 view_mat;
    glm::mat4 projection_mat;
    glm::mat4 cam_mat;
} Camera_t;

void Camera_init(Camera_t *camera, GLFWwindow *window, float fov, float near_plane, float far_plane);
// Note: This function opropable needs dt
void Camera_processInput(Camera_t *camera, GLFWwindow *window);

#endif // FAVE_CAMERA_H


#ifndef FAVE_CAMERA_H
#define FAVE_CAMERA_H

#include <cstdio>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifdef __cplusplus
extern "C"
{
#endif
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

        int first_input_click;

        glm::mat4 view_mat;
        glm::mat4 projection_mat;
        glm::mat4 cam_mat;
    } Camera_t;

    void Camera_init(Camera_t *camera, GLFWwindow *window, glm::vec3 postion, glm::vec3 rotation, float speed, float fov, float near_plane, float far_plane);
    // Note: This function opropable needs dt
    void Camera_processInput(Camera_t *camera, GLFWwindow *window);
    void Camera_destroy(Camera_t *camera);
#ifdef __cplusplus
}
#endif

#endif // FAVE_CAMERA_H

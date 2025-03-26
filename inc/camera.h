

#ifndef FAVE_CAMERA_H
#define FAVE_CAMERA_H

#include <cstdio>

#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <cglm/struct.h>
#include <GLFW/glfw3.h>
#include <math.h>

#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct Camera
    {
        vec3s position;
        vec3s direction;
        vec3s up;

        float fov;
        float near_plane;
        float far_plane;
        float speed;
        float sensitivity;

        int first_input_click;

        mat4s view_mat;
        mat4s projection_mat;
        mat4s cam_mat;
    } Camera_t;

    void Camera_init(Camera_t *camera, GLFWwindow *window, vec3s position, vec3s rotation, float speed, float fov, float near, float far);
    // Note: This function opropable needs dt
    void Camera_processInput(Camera_t *camera, GLFWwindow *window);
    void Camera_destroy(Camera_t *camera);
#ifdef __cplusplus
}
#endif

#endif // FAVE_CAMERA_H

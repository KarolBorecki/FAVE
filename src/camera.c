#include "camera.h"

void Camera_init(Camera_t *camera, GLFWwindow *window, vec3s position, vec3s rotation, float speed, float fov, float near, float far)
{
    int window_width, window_height;
    glfwGetWindowSize(window, &window_width, &window_height);

    camera->position = position;
    camera->direction = rotation;
    camera->up = (vec3s){{0.0f, 1.0f, 0.0f}};

    camera->fov = fov;
    camera->near_plane = near;
    camera->far_plane = far;
    camera->speed = speed;
    camera->sensitivity = 100.0f;

    camera->first_input_click = 0;

    glm_perspective(glm_rad(fov), (float)window_width / (float)window_height, near, far, camera->projection_mat.raw);
    glm_lookat(camera->position.raw,
               glms_vec3_add(camera->position, camera->direction).raw,
               camera->up.raw,
               camera->view_mat.raw);
    camera->cam_mat = glms_mat4_mul(camera->projection_mat, camera->view_mat);
}

void Camera_processInput(Camera_t *camera, GLFWwindow *window)
{
    int update_mat = 0;
    int window_width, window_height;
    glfwGetWindowSize(window, &window_width, &window_height);

    float camera_speed = camera->speed;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera->position = glms_vec3_add(camera->position, glms_vec3_scale(camera->direction, camera_speed));
        update_mat = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera->position = glms_vec3_sub(camera->position, glms_vec3_scale(camera->direction, camera_speed));
        update_mat = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        vec3s right = glms_vec3_normalize(glms_vec3_cross(camera->direction, camera->up));
        camera->position = glms_vec3_sub(camera->position, glms_vec3_scale(right, camera_speed));
        update_mat = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        vec3s right = glms_vec3_normalize(glms_vec3_cross(camera->direction, camera->up));
        camera->position = glms_vec3_add(camera->position, glms_vec3_scale(right, camera_speed));
        update_mat = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        camera->position = glms_vec3_add(camera->position, glms_vec3_scale(camera->up, camera_speed));
        update_mat = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        camera->position = glms_vec3_sub(camera->position, glms_vec3_scale(camera->up, camera_speed));
        update_mat = 1;
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        if (camera->first_input_click)
        {
            glfwSetCursorPos(window, window_width / 2, window_height / 2);
            camera->first_input_click = 0;
        }

        double mouse_x, mouse_y;
        glfwGetCursorPos(window, &mouse_x, &mouse_y);

        float rot_x = camera->sensitivity * (float)(mouse_y - (window_height / 2)) / window_height;
        float rot_y = camera->sensitivity * (float)(mouse_x - (window_width / 2)) / window_width;

        vec3s rotation_axis_x = glms_vec3_normalize(glms_vec3_cross(camera->direction, camera->up));
        versors quat_x = glms_quatv(glm_rad(-rot_x), rotation_axis_x);
        vec3s new_orientation = glms_quat_rotatev(quat_x, camera->direction);

        float dot_product = glms_vec3_dot(new_orientation, camera->up);
        if (fabsf(glm_deg(acosf(dot_product)) - 90.0f) <= 85.0f)
        {
            camera->direction = new_orientation;
        }

        versors quat_y = glms_quatv(glm_rad(-rot_y), camera->up);
        camera->direction = glms_quat_rotatev(quat_y, camera->direction);

        glfwSetCursorPos(window, window_width / 2, window_height / 2);
        update_mat = 1;
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        camera->first_input_click = 1;
    }

    if (update_mat)
    {
        glm_lookat(camera->position.raw,
                   glms_vec3_add(camera->position, camera->direction).raw,
                   camera->up.raw,
                   camera->view_mat.raw);
        camera->cam_mat = glms_mat4_mul(camera->projection_mat, camera->view_mat);
        printf("%lf %lf %lf\n", camera->position.raw[0], camera->position.raw[1], camera->position.raw[2]);
    }
}

void Camera_destroy(Camera_t *camera)
{
}
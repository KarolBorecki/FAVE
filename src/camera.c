#include "camera.h"

void Camera_init(Camera_t *camera, GLFWwindow *window, float fov, float near, float far)
{
    int window_width, window_height;
    glfwGetWindowSize(window, &window_width, &window_height);

    camera->position = glm::vec3(15.71f, 8.86f, 38.01f);
    camera->direction = glm::vec3(0.0f, 0.0f, -1.0f);
    camera->up = glm::vec3(0.0f, 1.0f, 0.0f);

    camera->fov = fov;
    camera->near_plane = near;
    camera->far_plane = far;
    camera->speed = 0.05f;
    camera->sensitivity = 100.0f;

    camera->first_input_click = 0;

    camera->projection_mat = glm::perspective(glm::radians(fov), (float)window_width / (float)window_height, near, far);
    camera->view_mat = glm::lookAt(camera->position, camera->position + camera->direction, camera->up);
    camera->cam_mat = camera->projection_mat * camera->view_mat;
}

void Camera_processInput(Camera_t *camera, GLFWwindow *window)
{
    uint8_t update_mat = 0;
    int window_width, window_height;
    glfwGetWindowSize(window, &window_width, &window_height);

    float camera_speed = camera->speed;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera->position += camera_speed * camera->direction;
        update_mat = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera->position -= camera_speed * camera->direction;
        update_mat = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera->position -= glm::normalize(glm::cross(camera->direction, camera->up)) * camera_speed;
        update_mat = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera->position += glm::normalize(glm::cross(camera->direction, camera->up)) * camera_speed;
        update_mat = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        camera->position += camera_speed * camera->up;
        update_mat = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        camera->position -= camera_speed * camera->up;
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

        glm::quat rotation_quat_x = glm::angleAxis(glm::radians(-rot_x), glm::normalize(glm::cross(camera->direction, camera->up)));
        glm::vec3 new_orientation = rotation_quat_x * camera->direction;

        float dot_product = glm::dot(new_orientation, camera->up);
        if (glm::abs(glm::degrees(glm::acos(dot_product)) - 90.0f) <= 85.0f)
        {
            camera->direction = new_orientation;
        }

        glm::quat rotation_quat_y = glm::angleAxis(glm::radians(-rot_y), camera->up);
        camera->direction = rotation_quat_y * camera->direction;

        glfwSetCursorPos(window, window_width / 2, window_height / 2);
        update_mat = 1;
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        camera->first_input_click = 1;
        update_mat = 1;
    }

    if (update_mat)
    {
        camera->view_mat = glm::lookAt(camera->position, camera->position + camera->direction, camera->up);
        camera->cam_mat = camera->projection_mat * camera->view_mat;
    }
}

void Camera_destroy(Camera_t *camera)
{
    // Nothing to do here
}

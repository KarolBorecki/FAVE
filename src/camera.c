#include "camera.h"

void processCameraInput(GLFWwindow *window, Camera &camera)
{
    int window_width, window_height;
    glfwGetWindowSize(window, &window_width, &window_height);

    float camera_speed = camera.speed;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.position += camera_speed * camera.direction;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.position -= camera_speed * camera.direction;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.position -= glm::normalize(glm::cross(camera.direction, camera.up)) * camera_speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.position += glm::normalize(glm::cross(camera.direction, camera.up)) * camera_speed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.position += camera_speed * camera.up;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.position -= camera_speed * camera.up;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        if (camera.first_input_click)
        {
            glfwSetCursorPos(window, window_width / 2, window_height / 2);
            camera.first_input_click = false;
        }

        double mouse_x, mouse_y;
        glfwGetCursorPos(window, &mouse_x, &mouse_y);

        float rot_x = camera.sensitivity * (float)(mouse_y - (window_height / 2)) / window_height;
        float rot_y = camera.sensitivity * (float)(mouse_x - (window_width / 2)) / window_width;

        glm::quat rotation_quat_x = glm::angleAxis(glm::radians(-rot_x), glm::normalize(glm::cross(camera.direction, camera.up)));
        glm::vec3 new_orientation = rotation_quat_x * camera.direction;

        float dot_product = glm::dot(new_orientation, camera.up);
        if (glm::abs(glm::degrees(glm::acos(dot_product)) - 90.0f) <= 85.0f)
        {
            camera.direction = new_orientation;
        }

        glm::quat rotation_quat_y = glm::angleAxis(glm::radians(-rot_y), camera.up);
        camera.direction = rotation_quat_y * camera.direction;

        glfwSetCursorPos(window, window_width / 2, window_height / 2);
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        camera.first_input_click = true;
    }
}

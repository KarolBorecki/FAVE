#include "objects/camera.h"

namespace FAVE
{
    Camera::Camera()
    {
        setPosition(glm::vec3(0.0f, 1.0f, 4.0f));
        setOrientation(glm::vec3(0.0f, 0.0f, -1.0f));
    }

    void Camera::update()
    {
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);
        view = glm::lookAt(m_position, m_position + m_rotation, m_up);
        float aspect_ratio = (float)m_width / (float)m_height;
        float f_fov_rad = glm::radians(m_fov);
        projection = glm::perspective(f_fov_rad, aspect_ratio, m_near_plane, m_far_plane);

        m_camera_matrix = projection * view;
    }

    void Camera::input(GLFWwindow *p_window, float deltaTime) // TODO rework
    {
        float camera_speed = m_speed;
        if (glfwGetKey(p_window, GLFW_KEY_W) == GLFW_PRESS)
            m_position += camera_speed * m_rotation;
        if (glfwGetKey(p_window, GLFW_KEY_S) == GLFW_PRESS)
            m_position -= camera_speed * m_rotation;
        if (glfwGetKey(p_window, GLFW_KEY_A) == GLFW_PRESS)
            m_position -= glm::normalize(glm::cross(m_rotation, m_up)) * camera_speed;
        if (glfwGetKey(p_window, GLFW_KEY_D) == GLFW_PRESS)
            m_position += glm::normalize(glm::cross(m_rotation, m_up)) * camera_speed;
        if (glfwGetKey(p_window, GLFW_KEY_SPACE) == GLFW_PRESS)
            m_position += m_up * camera_speed;
        if (glfwGetKey(p_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            m_position -= m_up * camera_speed;

        if (glfwGetMouseButton(p_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            glfwSetInputMode(p_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

            if (m_first_click)
            {
                glfwSetCursorPos(p_window, (m_width / 2), (m_height / 2));
                m_first_click = false;
            }

            double mouseX;
            double mouseY;
            glfwGetCursorPos(p_window, &mouseX, &mouseY);

            float rotX = m_sensitivity * (float)(mouseY - (m_height / 2)) / m_height;
            float rotY = m_sensitivity * (float)(mouseX - (m_width / 2)) / m_width;

            glm::vec3 new_orientation = glm::rotate(m_rotation, glm::radians(-rotX), glm::normalize(glm::cross(m_rotation, m_up)));

            if (abs(glm::angle(new_orientation, m_up) - glm::radians(90.0f)) <= glm::radians(85.0f))
            {
                m_rotation = new_orientation;
            }

            m_rotation = glm::rotate(m_rotation, glm::radians(-rotY), m_up);

            glfwSetCursorPos(p_window, (m_width / 2), (m_height / 2));
        }
        else if (glfwGetMouseButton(p_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
        {
            glfwSetInputMode(p_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            m_first_click = true;
        }
    }

    void Camera::matrix(Shader &shader, const char *uniform)
    {
        glUniformMatrix4fv(glGetUniformLocation(shader.id(), uniform), 1, GL_FALSE, glm::value_ptr(m_camera_matrix));
    }
}
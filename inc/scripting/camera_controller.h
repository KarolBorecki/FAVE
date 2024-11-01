#ifndef FAVE_CAMERA_CONTROLLER_H
#define FAVE_CAMERA_CONTROLLER_H

#include <glad/glad.h>

namespace FAVE
{
    class CameraController : public Script
    {
    public:
        CameraController(GLFWwindow *p_window) : m_window(p_window) {}

        void update(float deltaTime)
        {
            float camera_speed = m_speed;
            if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
                m_object->move(camera_speed * m_object->rotation());
            if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
                m_object->move(-camera_speed * m_object->rotation());
            if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
                m_object->move(-glm::normalize(glm::cross(m_object->rotation(), m_object->up())) * camera_speed);
            if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
                m_object->move(glm::normalize(glm::cross(m_object->rotation(), m_object->up())) * camera_speed);
            if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
                m_object->move(m_object->up() * camera_speed);
            if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                m_object->move(-m_object->up() * camera_speed);

            if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            {
                glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

                if (m_first_click)
                {
                    glfwSetCursorPos(m_window, (m_width / 2), (m_height / 2));
                    m_first_click = false;
                }

                double mouse_x;
                double mouse_y;
                glfwGetCursorPos(m_window, &mouse_x, &mouse_y);

                float rotX = m_sensitivity * (float)(mouse_y - (m_height / 2)) / m_height;
                float rotY = m_sensitivity * (float)(mouse_x - (m_width / 2)) / m_width;

                glm::vec3 new_orientation = glm::rotate(m_rotation, glm::radians(-rotX), glm::normalize(glm::cross(m_rotation, m_up)));

                if (abs(glm::angle(new_orientation, m_up) - glm::radians(90.0f)) <= glm::radians(85.0f))
                {
                    m_rotation = new_orientation;
                }

                m_rotation = glm::rotate(m_rotation, glm::radians(-rotY), m_up);

                glfwSetCursorPos(m_window, (m_width / 2), (m_height / 2));
            }
            else if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
            {
                glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                m_first_click = true;
            }
        }

    private:
        GLFWwindow *m_window;

        float m_speed = 0.01f;
        float m_sensitivity = 0.1f;

        bool m_firstClick = true;
    };
}

#endif
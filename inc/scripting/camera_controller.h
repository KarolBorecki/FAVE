#ifndef FAVE_CAMERA_CONTROLLER_H
#define FAVE_CAMERA_CONTROLLER_H

#include <glad/glad.h>

#include "logging/exceptions.h"

namespace FAVE
{
    class CameraController : public Script
    {
    public:
        using Script::Script;

        void start() override
        {
            if (m_camera == nullptr)
            {
                throwError("CameraController script requires a Camera component to be attached to the same GameObject.");
            }

            m_window = glfwGetCurrentContext();
            if (m_window == nullptr)
            {
                throwError("CameraController script requires a window to be created.");
            }
        }

        void update(float p_delta_time) override
        {
            float camera_speed = m_speed;
            if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
                m_camera->move(camera_speed * m_camera->rotation());
            if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
                m_camera->move(-camera_speed * m_camera->rotation());
            if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
                m_camera->move(-glm::normalize(glm::cross(m_camera->rotation(), m_camera->up())) * camera_speed);
            if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
                m_camera->move(glm::normalize(glm::cross(m_camera->rotation(), m_camera->up())) * camera_speed);
            if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
                m_camera->move(m_camera->up() * camera_speed);
            if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                m_camera->move(-m_camera->up() * camera_speed);

            if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            {
                glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

                if (m_first_click)
                {
                    glfwSetCursorPos(m_window, (m_camera->width() / 2), (m_camera->height() / 2));
                    m_first_click = false;
                }

                double mouse_x;
                double mouse_y;
                glfwGetCursorPos(m_window, &mouse_x, &mouse_y);

                float rot_x = m_sensitivity * (float)(mouse_y - (m_camera->height() / 2)) / m_camera->height();
                float rot_y = m_sensitivity * (float)(mouse_x - (m_camera->width() / 2)) / m_camera->width();

                glm::vec3 new_orientation = glm::rotate(m_camera->rotation(), glm::radians(-rot_x), glm::normalize(glm::cross(m_camera->rotation(), m_camera->up())));

                if (abs(glm::angle(new_orientation, m_camera->up()) - glm::radians(90.0f)) <= glm::radians(85.0f))
                {
                    m_camera->setOrientation(new_orientation);
                }

                m_camera->setOrientation(glm::rotate(m_camera->rotation(), glm::radians(-rot_y), m_camera->up()));

                glfwSetCursorPos(m_window, (m_camera->width() / 2), (m_camera->height() / 2));
            }
            else if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
            {
                glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                m_first_click = true;
            }
            // log("Camera position: %f, %f, %f", m_camera->position().x, m_camera->position().y, m_camera->position().z);
            // log("Camera rotation: %f, %f, %f", m_camera->rotation().x, m_camera->rotation().y, m_camera->rotation().z);
        }

    public:
        GLFWwindow *m_window{nullptr};
        Camera *m_camera{nullptr};

        float m_speed = 0.01f;
        float m_sensitivity = 80.0f;

    private:
        bool m_first_click = true;
    };
}

#endif
#include "objects/camera.h"

namespace FAVE
{
    Camera::Camera()
    {
        setPosition(glm::vec3(0.0f, 1.0f, 4.0f));
        setOrientation(glm::vec3(0.0f, 0.0f, -1.0f));
    }

    void Camera::update_matrix()
    {
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);
        view = glm::lookAt(m_position, m_position + m_rotation, m_up);
        float aspect_ratio = (float)m_width / (float)m_height;
        float f_fov_rad = glm::radians(m_fov);
        projection = glm::perspective(f_fov_rad, aspect_ratio, m_near_plane, m_far_plane);

        m_camera_matrix = projection * view;
    }

    void Camera::matrix(Shader &shader, const char *uniform)
    {
        glUniformMatrix4fv(glGetUniformLocation(shader.id(), uniform), 1, GL_FALSE, glm::value_ptr(m_camera_matrix));
    }
}
#ifndef FAVE_CAMERA_H
#define FAVE_CAMERA_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "objects/object.h"
#include "materials/shader.h"

namespace FAVE
{
    class Camera : public Object
    {
    public:
        Camera();
        virtual ~Camera() = default;

        void update();
        void input(GLFWwindow *window, float deltaTime);
        void matrix(Shader &shader, const char *uniform);

        inline void setSize(uint16_t p_width, uint16_t p_height) { m_width = p_width; m_height = p_height; }

        inline void setSpeed(float speed) { m_speed = speed; }
        inline void setSensitivity(float sensitivity) { m_sensitivity = sensitivity; }

    private:
        float m_fov = 45.0f;
        float m_near_plane = 0.1f;
        float m_far_plane = 100.0f;

        uint16_t m_width{1};
        uint16_t m_height{1};

        glm::mat4 m_camera_matrix = glm::mat4(1.0f);

        float m_speed = 0.01f;
        float m_sensitivity = 100.0f;

        const glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);

        bool m_first_click = true;
    };
}

#endif // FAVE_CAMERA_H
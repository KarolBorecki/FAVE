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

        void update_matrix();
        void matrix(Shader &shader, const char *uniform);

        inline void setSize(uint16_t p_width, uint16_t p_height)
        {
            m_width = p_width;
            m_height = p_height;
        }

        inline uint16_t width() { return m_width; }
        inline uint16_t height() { return m_height; }

    private:
        float m_fov = 45.0f;
        float m_near_plane = 0.1f;
        float m_far_plane = 100.0f;

        uint16_t m_width{1};
        uint16_t m_height{1};

        glm::mat4 m_camera_matrix = glm::mat4(1.0f);

        bool m_first_click = true;
    };
}

#endif // FAVE_CAMERA_H
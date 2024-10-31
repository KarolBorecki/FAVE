#ifndef FAVE_OBJECT_H
#define FAVE_OBJECT_H

#include <glm/glm.hpp>

namespace FAVE
{
    class Object
    {
    public:
        Object() = default;
        virtual ~Object() = default;

        inline void setScale(glm::vec3 p_scale) { m_scale = p_scale; }
        inline void setPosition(glm::vec3 p_position) { m_position = p_position; }
        inline void setOrientation(glm::vec3 p_orientation) { m_rotation = p_orientation; }

        inline void move(glm::vec3 p_move) { m_position += p_move; }
        inline void rotate(glm::vec3 p_rotate) { m_rotation += p_rotate; }

        inline glm::vec3 scale() const { return m_scale; }
        inline glm::vec3 position() const { return m_position; }
        inline glm::vec3 rotation() const { return m_rotation; }

    protected:
        glm::vec3 m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 m_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    };
}

#endif // FAVE_OBJECT_H
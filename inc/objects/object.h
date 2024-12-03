#ifndef FAVE_OBJECT_H
#define FAVE_OBJECT_H

#include <vector>
#include <glm/glm.hpp>

#include "scripting/script.h"

namespace FAVE
{
    class Object
    {
    public:
        Object() = default;

        inline void setScale(glm::vec3 p_scale) { m_scale = p_scale; }
        inline void setPosition(glm::vec3 p_position) { m_position = p_position; }
        inline void setOrientation(glm::vec3 p_orientation) { m_rotation = p_orientation; }

        inline void move(glm::vec3 p_move) { m_position += p_move; }
        inline void rotate(glm::vec3 p_rotate) { m_rotation += p_rotate; }

        inline glm::vec3 scale() const { return m_scale; }
        inline glm::vec3 position() const { return m_position; }
        inline glm::vec3 rotation() const { return m_rotation; }

        inline glm::vec3 up() const { return m_up; }

        void attach(Script *p_script)
        {
            m_scripts.push_back(p_script);
            p_script->attachTo(this);
        }

        void startScripts()
        {
            for (auto &script : m_scripts)
                script->start();
        }

        void updateScripts(float p_delta_time)
        {
            for (auto &script : m_scripts)
                script->update(p_delta_time);
        }

        void fixedUpdateScripts(float p_fixed_delta_time)
        {
            for (auto &script : m_scripts)
                script->fixedUpdate(p_fixed_delta_time);
        }

        virtual void start() {};
        virtual void update(float p_delta_time) {};
        virtual void fixedUpdate(float p_fixed_delta_time) {};

    protected:
        glm::vec3 m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 m_rotation = glm::vec3(0.0f, 0.0f, 0.0f); // Euler angles

        const glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);

        std::vector<Script *> m_scripts;
    };
}

#endif // FAVE_OBJECT_H
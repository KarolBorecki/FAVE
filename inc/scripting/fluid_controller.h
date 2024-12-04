#ifndef FAVE_FLUID_CONTROLLER_H
#define FAVE_FLUID_CONTROLLER_H

#include <glad/glad.h>

#include "logging/exceptions.h"
#include "objects/fluid_simulation.h"

namespace FAVE
{
    class FluidController : public Script
    {
    public:
        using Script::Script;

        void start() override
        {
            if (m_fluid == nullptr)
            {
                throwError("FluidController script requires a Fluid component to be attached.");
            }

            m_window = glfwGetCurrentContext();
            if (m_window == nullptr)
            {
                throwError("FluidController script requires a window to be created.");
            }
        }

        void update(float p_delta_time) override
        {
            if (glfwGetKey(m_window, GLFW_KEY_O) == GLFW_PRESS)
            {
                log("O key pressed");
                m_fluid->applyForce({0.0f, m_force}, {50.0f, 50.0f}, m_radius);
            }
        }

    public:
        GLFWwindow *m_window{nullptr};
        FluidSimulation *m_fluid{nullptr};

        float m_force = 7.0f;
        float m_radius = 10.0f;
    };
}

#endif // FAVE_FLUID_CONTROLLER_H
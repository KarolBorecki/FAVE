#ifndef FAVE_FLUID_SIMULATION_H
#define FAVE_FLUID_SIMULATION_H

#include <stdlib.h>
#include <cmath>
#include <time.h>
#include <chrono>
#include <array>
#include <vector>

#include <glm/glm.hpp>

#include "buffers/vao.h"
#include "buffers/vbo.h"
#include "buffers/ebo.h"
#include "objects/camera.h"
#include "objects/light.h"
#include "objects/render_object.h"
#include "materials/material.h"

namespace FAVE
{
    typedef struct GridCell
    {
        float p;
        float v, u, w; // v - up/down vec, u - left/right vec, w - forward/backward vec
        float new_v, new_u, new_w;
        float s;
        float new_s;
        float m;
        float new_m;
    } GridCell_t;

    typedef struct Marker
    {
        glm::vec3 position;
        glm::vec3 velocity;
        float pressure;
    } Marker_t;

    typedef enum FielType : uint8_t
    {
        U_field = 0,
        V_field = 1,
        W_field = 2,
        S_field = 3
    } FieldType_t;

    // TODO rename to GridFluidSimulation
    class FluidSimulation : public RenderObject
    {
    public:
        FluidSimulation(Material &p_material, float p_fluid_density, float p_grid_size) : m_material(p_material), m_fluid_density(p_fluid_density), m_grid_size(p_grid_size)
        {
            m_window = glfwGetCurrentContext();
        }

        void draw(float p_delta_time, Camera *p_camera, Light *p_light) override;

        void destroy() override
        {
            m_vao.destroy();
        }

        virtual void applyForce(glm::vec2 p_force, glm::vec2 p_position, float p_radius) = 0;

    protected:
        GLFWwindow *m_window{nullptr};
        Material &m_material;

        VAO m_vao;
        std::vector<Vertex> m_vertices;
        std::vector<GLuint> m_indices;

        float m_fluid_density = 1000.0f;

        float m_grid_size = 0.5f;

        static constexpr float GRAVITY{-9.8f};
    };
}

#endif // FAVE_FLUID_SIMULATION_H
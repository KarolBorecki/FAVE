#ifndef FAVE_FLUID_SIMULATION_H
#define FAVE_FLUID_SIMULATION_H

#include <array>
#include <cmath>
#include <chrono>
#include <time.h>
#include <stdlib.h>

#include "buffers/vao.h"
#include "buffers/vbo.h"
#include "buffers/ebo.h"
#include "objects/camera.h"
#include "objects/light.h"
#include "objects/render_object.h"
#include "materials/material.h"

namespace FAVE
{
    class FluidSimulation : public RenderObject
    {
    public:
        FluidSimulation(Material &p_material, uint16_t p_size_x, uint16_t p_size_y, uint16_t p_size_z, uint16_t p_water_level, float p_grid_size);
        virtual ~FluidSimulation();

        void draw(Camera *p_camera, Light *p_light);
        void update(float deltaTime);

        void destroy();

    private:
        Material &m_material;
        uint16_t m_size_x;
        uint16_t m_size_y;
        uint16_t m_size_z;
        uint16_t m_water_level;

        float m_grid_size = 1.0f;

        float ***m_scalar_field;
        std::vector<std::vector<std::vector<glm::vec3>>> m_velocities;
        glm::vec3 m_gravity = glm::vec3(0.0f, -9.8f, 0.0f);

        glm::vec3 m_sphereCenter = glm::vec3(10.0f, 10.0f, 10.0f);
        float m_sphereRadius = 5.0f;
        float m_collisionDamping = 0.9f;

        VAO m_vao;
        std::vector<Vertex> m_vertices;
        std::vector<GLuint> m_indices;

        void recognise_geometry();
        float calculatePressure(uint16_t x, uint16_t y, uint16_t z);
    };
}

#endif // FAVE_FLUID_SIMULATION_H
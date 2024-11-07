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
#include "objects/render_object.h"
#include "materials/material.h"

namespace FAVE
{
    class FluidSimulation : public RenderObject
    {
    public:
        FluidSimulation(Material &p_material, uint16_t p_size_x, uint16_t p_size_y, uint16_t p_size_z, uint16_t p_water_level, float p_cube_size);
        virtual ~FluidSimulation();

        void draw(Camera *p_camera, Light *p_light);

        void destroy();

        void update(float deltaTime);

    private:
        Material &m_material;
        uint16_t m_size_x;
        uint16_t m_size_y;
        uint16_t m_size_z;
        uint16_t m_water_level;

        float m_cube_size;

        bool ***m_cubes; // japierdole
        float ***m_scalar_field;
        std::vector<std::vector<std::vector<glm::vec3>>> m_velocities;
        glm::vec3 m_gravity = glm::vec3(0.0f, -9.8f, 0.0f);

        glm::vec3 m_sphereCenter = glm::vec3(10.0f, 10.0f, 10.0f);
        float m_sphereRadius = 5.0f;
        float m_collisionDamping = 0.8f;

        VAO m_vao;
        std::vector<Vertex> m_vertices;
        std::vector<GLuint> m_indices;

        float m_waveFrequency = 0.1f;
        float m_waveAmplitude = 1.0f;
        float m_time = 0.0f;

        void recognise_geometry();
        void add_face(const glm::vec3 &pos, const glm::vec3 &normal, const std::array<glm::vec3, 4> &offsets, uint16_t &vertex_count);
    };
}

#endif // FAVE_FLUID_SIMULATION_H
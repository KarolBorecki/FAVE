#include "objects/fluid_simulation.h"
#include "objects/camera.h"
#include "objects/light.h"
#include <glm/glm.hpp>

#include <glm/glm.hpp>
#include <vector>
#include <array>

namespace FAVE
{
    FluidSimulation::FluidSimulation(Material &p_material, uint16_t p_size_x, uint16_t p_size_y, uint16_t p_size_z, uint16_t p_water_level, float p_grid_size)
        : m_material(p_material), m_size_x(p_size_x), m_size_y(p_size_y), m_size_z(p_size_z),
          m_water_level(p_water_level), m_grid_size(p_grid_size)
    {
        // Initialize scalar field and velocities
        m_scalar_field = new float **[m_size_x];
        m_velocities.resize(m_size_x, std::vector<std::vector<glm::vec3>>(m_size_y, std::vector<glm::vec3>(m_size_z, glm::vec3(0.0f))));

        for (uint16_t x = 0; x < m_size_x; ++x)
        {
            m_scalar_field[x] = new float *[m_size_y];
            for (uint16_t y = 0; y < m_size_y; ++y)
            {
                m_scalar_field[x][y] = new float[m_size_z];
                for (uint16_t z = 0; z < m_size_z; ++z)
                {
                    m_scalar_field[x][y][z] = (y < m_water_level) ? 1.0f : 0.0f; // 1.0f for water, 0.0f for empty
                }
            }
        }
    }

    FluidSimulation::~FluidSimulation()
    {
        destroy();
    }

    void FluidSimulation::update(float deltaTime)
    {
        RenderObject::update(deltaTime);
        // Update sphere position
        m_sphereCenter += m_gravity * deltaTime;

        // Compute pressure gradient and update fluid behavior
        for (uint16_t x = 1; x < m_size_x - 1; ++x)
        {
            for (uint16_t y = 1; y < m_size_y - 1; ++y)
            {
                for (uint16_t z = 1; z < m_size_z - 1; ++z)
                {
                    if (m_scalar_field[x][y][z] == 1.0f)
                    { // Only consider water voxels

                        // Calculate pressure (using a simplified method like height-based pressure or using a pressure field)
                        float pressure = calculatePressure(x, y, z);

                        // Compute pressure gradient (simplified using central differences)
                        glm::vec3 pressureGradient = glm::vec3(0.0f);
                        pressureGradient.x = (pressure - calculatePressure(x - 1, y, z)) / m_grid_size;
                        pressureGradient.y = (pressure - calculatePressure(x, y - 1, z)) / m_grid_size;
                        pressureGradient.z = (pressure - calculatePressure(x, y, z - 1)) / m_grid_size;

                        // Update velocity based on pressure gradient and fluid viscosity
                        glm::vec3 velocityChange = -pressureGradient; // Using a simple pressure force model
                        m_velocities[x][y][z] += velocityChange;

                        // Apply gravity to water voxels
                        m_velocities[x][y][z] += m_gravity;

                        // Move voxel based on velocity
                        glm::vec3 pos = glm::vec3(x, y, z) * m_grid_size;
                        glm::vec3 new_pos = pos + m_velocities[x][y][z] * deltaTime;

                        // Update scalar field based on the new position
                        // If the voxel moves above the water level, it's no longer water, otherwise it is
                        if (new_pos.y > m_water_level * m_grid_size)
                        {
                            m_scalar_field[x][y][z] = 0.0f; // voxel is no longer water
                        }
                        else
                        {
                            m_scalar_field[x][y][z] = 1.0f; // voxel is water
                        }

                        // Boundary checking
                        if (new_pos.y < 0)
                        {
                            m_velocities[x][y][z].y = 0; // Reflect velocity on the ground
                        }

                        // Sphere collision handling
                        float distanceToSphere = glm::length(new_pos - m_sphereCenter);
                        if (distanceToSphere < m_sphereRadius)
                        {
                            // Reflect velocity based on collision
                            glm::vec3 collisionNormal = glm::normalize(new_pos - m_sphereCenter);
                            m_velocities[x][y][z] -= 2.0f * glm::dot(m_velocities[x][y][z], collisionNormal) * collisionNormal;
                            // Damping factor applied to simulate energy loss on collision
                            m_velocities[x][y][z] *= m_collisionDamping;

                            log("Collision detected at: [%lf, %lf, %lf] velocity is: [%lf, %lf, %lf]",
                                new_pos.x, new_pos.y, new_pos.z, m_velocities[x][y][z].x, m_velocities[x][y][z].y, m_velocities[x][y][z].z);
                        }
                    }
                }
            }
        }
    }

    // This function calculates the pressure at each voxel based on its height or density
    float FluidSimulation::calculatePressure(uint16_t x, uint16_t y, uint16_t z)
    {
        // Use the y-coordinate for a simple pressure model (higher y = higher pressure)
        // In a real simulation, this would likely depend on density or other factors.
        if (m_scalar_field[x][y][z] == 1.0f)
        {
            return glm::max(0.0f, float(m_water_level - y) * 0.1f); // Simple pressure gradient based on depth
        }
        return 0.0f;
    }

    void FluidSimulation::draw(Camera *p_camera, Light *p_light)
    {
        recognise_geometry();
        m_vao.bind();
        VBO vbo(m_vertices);
        EBO ebo(m_indices);
        m_vao.linkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void *)0);
        m_vao.linkAttrib(vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void *)(3 * sizeof(float)));
        m_vao.linkAttrib(vbo, 2, 3, GL_FLOAT, sizeof(Vertex), (void *)(6 * sizeof(float)));
        m_vao.linkAttrib(vbo, 3, 2, GL_FLOAT, sizeof(Vertex), (void *)(9 * sizeof(float)));
        m_vao.unbind();
        vbo.unbind();
        ebo.unbind();

        vbo.destroy();
        ebo.destroy();

        m_material.shader().use();

        glUniform3f(glGetUniformLocation(m_material.shader().id(), "scale"), m_scale.x, m_scale.y, m_scale.z);
        glUniform3f(glGetUniformLocation(m_material.shader().id(), "rotation"), m_rotation.x, m_rotation.y, m_rotation.z);
        glUniform3f(glGetUniformLocation(m_material.shader().id(), "position"), position().x, position().y, position().z);

        glUniform4f(glGetUniformLocation(m_material.shader().id(), "lightColor"), p_light->color().r, p_light->color().g, p_light->color().b, p_light->color().a);
        glUniform3f(glGetUniformLocation(m_material.shader().id(), "lightPos"), p_light->position().x, p_light->position().y, p_light->position().z);
        glUniform3f(glGetUniformLocation(m_material.shader().id(), "camPos"), p_camera->position().x, p_camera->position().y, p_camera->position().z);

        p_camera->matrix(m_material.shader(), "camMatrix");

        m_vao.bind();
        glUniform1i(glGetUniformLocation(m_material.shader().id(), "useTexture"), 0);

        glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
    }

    void FluidSimulation::destroy() // TODO
    {
        // for (uint16_t x = 0; x < m_size_x; ++x)
        // {
        //     for (uint16_t y = 0; y < m_size_y; ++y)
        //     {
        //         delete[] m_scalar_field[x][y];
        //     }
        //     delete[] m_scalar_field[x];
        // }
        // delete[] m_scalar_field;
    }

    void FluidSimulation::recognise_geometry()
    {
        m_vertices.clear();
        m_indices.clear();

        const std::array<glm::vec3, 8> cubeVertices = {
            glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f),
            glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, -0.5f),
            glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f),
            glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f)};

        const std::array<unsigned int, 36> cubeIndices = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4,
            4, 0, 3, 3, 7, 4,
            1, 5, 6, 6, 2, 1,
            3, 2, 6, 6, 7, 3,
            4, 5, 1, 1, 0, 4};

        for (uint16_t x = 0; x < m_size_x; ++x)
        {
            for (uint16_t y = 0; y < m_size_y; ++y)
            {
                for (uint16_t z = 0; z < m_size_z; ++z)
                {
                    if (m_scalar_field[x][y][z] == 1.0f)
                    {
                        glm::vec3 cubePos = glm::vec3(x, y, z) * m_grid_size;

                        for (const auto &vert : cubeVertices)
                        {
                            Vertex vertex;
                            vertex.position = cubePos + vert * m_grid_size;
                            vertex.color = glm::vec3(0.0f, 0.0f, 1.0f);
                            vertex.normal = glm::normalize(vert);
                            m_vertices.push_back(vertex);
                        }

                        unsigned int offset = m_vertices.size() - cubeVertices.size();
                        for (const auto &idx : cubeIndices)
                        {
                            m_indices.push_back(offset + idx);
                        }
                    }
                }
            }
        }
    }

}

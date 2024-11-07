#include "objects/fluid_simulation.h"
#include "objects/camera.h"
#include "objects/light.h"
#include <glm/glm.hpp>

namespace FAVE
{

    FluidSimulation::FluidSimulation(Material &p_material, uint16_t p_size_x, uint16_t p_size_y, uint16_t p_size_z, uint16_t p_water_level, float p_cube_size)
        : m_material(p_material), m_size_x(p_size_x), m_size_y(p_size_y), m_size_z(p_size_z),
          m_water_level(p_water_level), m_cube_size(p_cube_size)
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
        // Update sphere position
        m_sphereCenter += m_gravity * deltaTime;

        // Update scalar field with fluid movement and sphere collision
        for (uint16_t x = 0; x < m_size_x; ++x)
        {
            for (uint16_t y = 0; y < m_size_y; ++y)
            {
                for (uint16_t z = 0; z < m_size_z; ++z)
                {
                    glm::vec3 pos = glm::vec3(x, y, z) * m_cube_size;

                    // Check if within sphere collision radius
                    if (glm::length(pos - m_sphereCenter) < m_sphereRadius)
                    {
                        m_velocities[x][y][z] *= -m_collisionDamping; // Reflect velocity
                        log("Collision detected at: [%lf, %lf, %lf] velocity is: [%lf, %lf, %lf]", pos.x, pos.y, pos.z, m_velocities[x][y][z].x, m_velocities[x][y][z].y, m_velocities[x][y][z].z);
                    }

                    // Apply gravity to water voxels
                    if (m_scalar_field[x][y][z] == 1.0f)
                    {
                        m_velocities[x][y][z] += m_gravity;
                    }

                    // Move voxel based on velocity
                    glm::vec3 new_pos = pos + m_velocities[x][y][z] * deltaTime;
                    // Implement boundary checking and update scalar field here if needed
                }
            }
        }
    }

    void FluidSimulation::draw(Camera *p_camera, Light *p_light)
    {
        update(0.016f);
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

    void FluidSimulation::destroy()
    {
        for (uint16_t x = 0; x < m_size_x; ++x)
        {
            for (uint16_t y = 0; y < m_size_y; ++y)
            {
                delete[] m_scalar_field[x][y];
            }
            delete[] m_scalar_field[x];
        }
        delete[] m_scalar_field;
    }

    void FluidSimulation::recognise_geometry()
    {
        m_vertices.clear();
        m_indices.clear();

        // Wzorzec dla sześcianu - 8 wierzchołków i 6 ścian (każda składa się z 2 trójkątów)
        const std::array<glm::vec3, 8> cubeVertices = {
            glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f),
            glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, -0.5f),
            glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f),
            glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f)};

        const std::array<unsigned int, 36> cubeIndices = {
            0, 1, 2, 2, 3, 0, // back face
            4, 5, 6, 6, 7, 4, // front face
            4, 0, 3, 3, 7, 4, // left face
            1, 5, 6, 6, 2, 1, // right face
            3, 2, 6, 6, 7, 3, // top face
            4, 5, 1, 1, 0, 4  // bottom face
        };

        // Generate geometry for each voxel with water
        for (uint16_t x = 0; x < m_size_x; ++x)
        {
            for (uint16_t y = 0; y < m_size_y; ++y)
            {
                for (uint16_t z = 0; z < m_size_z; ++z)
                {
                    if (m_scalar_field[x][y][z] == 1.0f)
                    {
                        glm::vec3 cubePos = glm::vec3(x, y, z) * m_cube_size;

                        // Add vertices for the current voxel cube
                        for (const auto &vert : cubeVertices)
                        {
                            Vertex vertex;
                            vertex.position = cubePos + vert * m_cube_size; // Position scaled by cube size
                            vertex.color = glm::vec3(0.0f, 0.0f, 1.0f);     // Set a color, e.g., blue for water
                            vertex.normal = glm::normalize(vert);           // Approximate normals
                            m_vertices.push_back(vertex);
                        }

                        // Add indices, offset by the current number of vertices
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

} // namespace FAVE

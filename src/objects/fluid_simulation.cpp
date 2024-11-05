#include "objects/fluid_simulation.h"
#include "util/marching_cubes.h"
#include <cmath>
#include <chrono>
#include <time.h>
#include <stdlib.h>

namespace FAVE
{
    FluidSimulation::FluidSimulation(Material &p_material, uint16_t p_size_x, uint16_t p_size_y, uint16_t p_size_z, uint16_t p_water_level, float p_cube_size)
        : m_material(p_material), m_size_x(p_size_x), m_size_y(p_size_y), m_size_z(p_size_z), m_water_level(p_water_level), m_cube_size(p_cube_size)
    {
        srand(time(NULL));
        m_scalar_field = new float **[m_size_x];
        for (uint16_t x = 0; x < m_size_x; x++)
        {
            m_scalar_field[x] = new float *[m_size_y];
            for (uint16_t y = 0; y < m_size_y; y++)
            {
                m_scalar_field[x][y] = new float[m_size_z];
                for (uint16_t z = 0; z < m_size_z; z++)
                {
                    m_scalar_field[x][y][z] = (y <= m_water_level) ? -1.0f : 1.0f;
                    // log("Scalar field at %d, %d, %d: %lf", x, y, z, m_scalar_field[x][y][z]);
                }
            }
        }
        log("Created fluid simulation with Marching Cubes with size %d, %d, %d and water level %d", m_size_x, m_size_y, m_size_z, m_water_level);
    }

    FluidSimulation::~FluidSimulation()
    {
        for (uint16_t x = 0; x < m_size_x; x++)
        {
            for (uint16_t y = 0; y < m_size_y; y++)
            {
                delete[] m_scalar_field[x][y];
            }
            delete[] m_scalar_field[x];
        }
        delete[] m_scalar_field;
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
    }

    void FluidSimulation::recognise_geometry()
    {
        m_vertices.clear();
        m_indices.clear();

        for (uint16_t x = 0; x < m_size_x - 1; x++)
        {
            for (uint16_t y = 0; y < m_size_y - 1; y++)
            {
                for (uint16_t z = 0; z < m_size_z - 1; z++)
                {
                    glm::vec3 voxel_pos(x * m_cube_size, y * m_cube_size, z * m_cube_size);

                    float corners[8] = {
                        m_scalar_field[x][y][z],
                        m_scalar_field[x + 1][y][z],
                        m_scalar_field[x + 1][y + 1][z],
                        m_scalar_field[x][y + 1][z],
                        m_scalar_field[x][y][z + 1],
                        m_scalar_field[x + 1][y][z + 1],
                        m_scalar_field[x + 1][y + 1][z + 1],
                        m_scalar_field[x][y + 1][z + 1]};

                    int cube_index = 0;
                    for (int i = 0; i < 8; i++)
                    {
                        if (corners[i] < 0)
                            cube_index |= (1 << i);
                    }

                    const int *triangulation = marching_cubes::triTable[cube_index];

                    for (int i = 0; triangulation[i] != -1; i += 3)
                    {
                        glm::vec3 vertex_positions[3];
                        for (int j = 0; j < 3; ++j)
                        {
                            int edge = triangulation[i + j];
                            glm::vec3 p1 = voxel_pos + marching_cubes::edgeVertexOffsets[edge][0] * m_cube_size;
                            glm::vec3 p2 = voxel_pos + marching_cubes::edgeVertexOffsets[edge][1] * m_cube_size;

                            float val1 = corners[marching_cubes::cornerIndexAFromEdge[edge]];
                            float val2 = corners[marching_cubes::cornerIndexBFromEdge[edge]];
                            vertex_positions[j] = p1 + (p2 - p1) * (val1 / (val1 - val2));
                        }

                        unsigned int baseIndex = static_cast<unsigned int>(m_vertices.size());
                        for (int j = 0; j < 3; j++)
                        {
                            glm::vec3 color(0.0f, 0.4f, 1.0f);
                            glm::vec3 normal = glm::normalize(glm::cross(vertex_positions[1] - vertex_positions[0], vertex_positions[2] - vertex_positions[0]));
                            m_vertices.push_back(Vertex{vertex_positions[j], normal, color, glm::vec2(0.0f, 0.0f)});
                            m_indices.push_back(baseIndex + j);
                        }
                    }
                }
            }
        }
    }

    void FluidSimulation::update(float deltaTime)
    {
        m_time += deltaTime;

        for (uint16_t x = 0; x < m_size_x; x++)
        {
            for (uint16_t y = 0; y < m_size_y; y++)
            {
                for (uint16_t z = 0; z < m_size_z; z++)
                {
                    if (y <= m_water_level)

                    {
                        int r = (0 + rand() % 1000) / 1000;
                        m_scalar_field[x][y][z] = m_waveAmplitude * sin(m_waveFrequency * ((x + r) + m_time * 5.0f)) - 1.0f;
                    }
                }
            }
        }
    }
}

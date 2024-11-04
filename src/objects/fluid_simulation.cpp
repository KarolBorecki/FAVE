#include "objects/fluid_simulation.h"

namespace FAVE
{
    FluidSimulation::FluidSimulation(Material &p_material, uint16_t p_size_x, uint16_t p_size_y, uint16_t p_size_z, uint16_t p_water_level, float p_cube_size)
        : m_material(p_material), m_size_x(p_size_x), m_size_y(p_size_y), m_size_z(p_size_z), m_water_level(p_water_level), m_cube_size(p_cube_size)
    {
        m_cubes = new bool **[m_size_x];
        for (uint16_t x = 0; x < m_size_x; x++)
        {
            m_cubes[x] = new bool *[m_size_y];
            for (uint16_t y = 0; y < m_size_y; y++)
            {
                m_cubes[x][y] = new bool[m_size_z];
                for (uint16_t z = 0; z < m_size_z; z++)
                {
                    m_cubes[x][y][z] = y <= m_water_level;
                }
            }
        }

        log("Created fluid simulation with size %dx%dx%d, water level %d and cube size %f", m_size_x, m_size_y, m_size_z, m_water_level, m_cube_size);
    }

    FluidSimulation::~FluidSimulation()
    {
        for (uint16_t x = 0; x < m_size_x; x++)
        {
            for (uint16_t y = 0; y < m_size_y; y++)
            {
                delete[] m_cubes[x][y];
            }
            delete[] m_cubes[x];
        }
        delete[] m_cubes;
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
        m_material.diffuseTexture()->texUnit(m_material.shader(), "diffuse0", 0);
        m_material.diffuseTexture()->bind();
        m_material.diffuseTexture()->texUnit(m_material.shader(), "specular0", 1);
        m_material.diffuseTexture()->bind();

        glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
        // glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    void FluidSimulation::destroy()
    {
        m_vertices.clear();
        m_indices.clear();
    }
    void FluidSimulation::recognise_geometry()
    {
        uint16_t vertex_count = 0;
        m_vertices.clear();
        m_indices.clear();

        for (uint16_t x = 0; x < m_size_x; x++)
        {
            for (uint16_t y = 0; y < m_size_y; y++)
            {
                for (uint16_t z = 0; z < m_size_z; z++)
                {
                    if (!m_cubes[x][y][z])
                        continue;

                    glm::vec3 pos(x * m_cube_size, y * m_cube_size, z * m_cube_size);

                    // Define vertices for each face with explicit std::array creation
                    add_face(pos, glm::vec3(0, 1, 0), std::array<glm::vec3, 4>{glm::vec3(0, 1, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 0), glm::vec3(1, 0, 0)}, vertex_count);

                    add_face(pos, glm::vec3(0, -1, 0), std::array<glm::vec3, 4>{glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec3(1, 0, -1), glm::vec3(1, 0, 0)}, vertex_count);

                    add_face(pos, glm::vec3(1, 0, 0), std::array<glm::vec3, 4>{glm::vec3(1, 0, 0), glm::vec3(1, 1, 0), glm::vec3(1, 1, -1), glm::vec3(1, 0, -1)}, vertex_count);

                    add_face(pos, glm::vec3(-1, 0, 0), std::array<glm::vec3, 4>{glm::vec3(0, 1, 0), glm::vec3(0, 1, -1), glm::vec3(0, 0, -1), glm::vec3(0, 0, 0)}, vertex_count);

                    add_face(pos, glm::vec3(0, 0, 1), std::array<glm::vec3, 4>{glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(1, 1, 0), glm::vec3(0, 1, 0)}, vertex_count);

                    add_face(pos, glm::vec3(0, 0, -1), std::array<glm::vec3, 4>{glm::vec3(0, 0, -1), glm::vec3(1, 0, -1), glm::vec3(1, 1, -1), glm::vec3(0, 1, -1)}, vertex_count);
                }
            }
        }

        log("Recognised geometry with %lu vertices and %lu indices", m_vertices.size(), m_indices.size());
    }

    void FluidSimulation::add_face(const glm::vec3 &pos, const glm::vec3 &normal, const std::array<glm::vec3, 4> &offsets, uint16_t &vertex_count)
    {
        glm::vec3 color = glm::vec3(0.4f, 0.7f, 1.0f); // Default water color

        for (auto &offset : offsets)
        {
            glm::vec3 vertex_pos = pos + offset * m_cube_size;
            m_vertices.push_back(Vertex{vertex_pos, normal, color, glm::vec2(0.0f, 0.0f)});
        }

        // Define indices for two triangles
        m_indices.push_back(vertex_count);
        m_indices.push_back(vertex_count + 1);
        m_indices.push_back(vertex_count + 2);
        m_indices.push_back(vertex_count + 2);
        m_indices.push_back(vertex_count + 3);
        m_indices.push_back(vertex_count);

        vertex_count += 4;
    }
}

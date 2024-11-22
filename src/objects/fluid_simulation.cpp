#include "objects/fluid_simulation.h"
#include "objects/camera.h"
#include "objects/light.h"
#include <glm/glm.hpp>

#include <vector>
#include <array>

namespace FAVE
{
    FluidSimulation::FluidSimulation(Material &p_material, uint16_t p_size_x, uint16_t p_size_y, uint16_t p_size_z, uint16_t p_water_level, float p_grid_size)
        : m_material(p_material), m_size_x(p_size_x), m_size_y(p_size_y), m_size_z(p_size_z), m_water_level(p_water_level), m_grid_size(p_grid_size)
    {
        m_cells = new GridCell **[m_size_x];
        for (uint16_t x = 0; x < m_size_x; ++x)
        {
            m_cells[x] = new GridCell *[m_size_y];
            for (uint16_t y = 0; y < m_size_y; ++y)
            {
                m_cells[x][y] = new GridCell[m_size_z];
                for (uint16_t z = 0; z < m_size_z; ++z)
                {
                    m_cells[x][y][z].u = 0.0f;
                    m_cells[x][y][z].v = 0.0f;
                    m_cells[x][y][z].w = 0.0f;

                    m_cells[x][y][z].s = y > m_water_level ? 0.0f : 1.0f;
                    if (x == 0 || x == m_size_x - 1 || y == 0 || y == m_size_y - 1 || z == 0 || z == m_size_z - 1)
                        m_cells[x][y][z].s = 0.0f; // borders

                    m_cells[x][y][z].p = 0.0f;

                    // log("x: %d y: %d z: %d", x, y, z);
                    // log("u: (%lf, %lf, %lf)", m_cells[x][y][z].u.x, m_cells[x][y][z].u.y, m_cells[x][y][z].u.z);
                    // log("v: (%lf, %lf, %lf)", m_cells[x][y][z].v.x, m_cells[x][y][z].v.y, m_cells[x][y][z].v.z);
                    // log("w: (%lf, %lf, %lf)", m_cells[x][y][z].w.x, m_cells[x][y][z].w.y, m_cells[x][y][z].w.z);
                    // log("s: %lf", m_cells[x][y][z].s);
                    // log("p: %lf", m_cells[x][y][z].p);
                }
            }
        }
    }

    FluidSimulation::~FluidSimulation()
    {
        destroy();
    }

    void FluidSimulation::update_physics(float p_delta_time)
    {
        // integrate
        for (uint16_t i = 0; i < m_size_x; ++i)
        {
            for (uint16_t j = 0; j < m_size_y; ++j)
            {
                for (uint16_t k = 0; k < m_size_z; ++k)
                {
                    m_cells[i][j][k].v += p_delta_time * -9.81f; // TODO gravity
                }
            }
        }

        // solve incompresabillity
        float c_p = m_fluid_density * m_grid_size / p_delta_time;
        for (uint16_t n = 0; n < m_solver_steps; n++)
        {
            for (uint16_t i = 1; i < m_size_x - 1; i++)
            {
                for (uint16_t j = 1; j < m_size_y - 1; j++)
                {
                    for (uint16_t k = 1; k < m_size_z - 1; k++)
                    {
                        if (m_cells[i][j][k].s == 0.0f)
                            continue;
                        float s = m_cells[i + 1][j][k].s + m_cells[i - 1][j][k].s + m_cells[i][j + 1][k].s + m_cells[i][j - 1][k].s + m_cells[i][j][k + 1].s + m_cells[i][j][k - 1].s;
                        if (s == 0.0f)
                            continue;

                        float d = m_over_relaxation * (m_cells[i + 1][j][k].u - m_cells[i][j][k].u + m_cells[i][j + 1][k].v - m_cells[i][j][k].v + m_cells[i][j][k + 1].w - m_cells[i][j][k].w);
                        float p = -d / s;
                        m_cells[i][j][k].u = m_cells[i][j][k].u - m_cells[i][j][k].s * p;
                        m_cells[i + 1][j][k].u = m_cells[i + 1][j][k].u + m_cells[i + 1][j][k].s * p;
                        m_cells[i][j][k].v = m_cells[i][j][k].v - m_cells[i][j][k].s * p;
                        m_cells[i][j + 1][k].v = m_cells[i][j + 1][k].v + m_cells[i][j + 1][k].s * p;
                        m_cells[i][j][k].w = m_cells[i][j][k].w - m_cells[i][j][k].s * p;
                        m_cells[i][j][k + 1].w = m_cells[i][j][k + 1].w + m_cells[i][j][k + 1].s * p;

                        m_cells[i][j][k].p += p * c_p; // TODO what is d_s, previosly was p=d_s
                    }
                }
            }
        }

        // advect
        // uint16_t n = m_size_y;
        float h = m_grid_size;
        float h2 = 0.5f * h;
        for (uint16_t i = 1; i < m_size_x - 1; i++)
        {
            for (uint16_t j = 1; j < m_size_y - 1; j++)
            {
                for (uint16_t k = 1; k < m_size_z - 1; k++)
                {
                    if (m_cells[i][j][k].s != 0.0f && m_cells[i - 1][j][k].s && j < m_size_y - 1 && k < m_size_z - 1)
                    {
                        float x = i * h;
                        float y = j * h + h2;
                        float z = k * h + h2;
                        float u = m_cells[i][j][k].u;
                        float v = avg_v(i, j, k);
                        float w = m_cells[i][j][k].w;

                        x = x - p_delta_time * u;
                        y = y - p_delta_time * v;
                        z = z - p_delta_time * w;
                        u = sample_field(x, y, z, 0);

                        m_cells[i][j][k].new_u = u;
                    }

                    if (m_cells[i][j][k].s != 0.0f && m_cells[i][j - 1][k].s && i < m_size_x - 1 && k < m_size_z - 1)
                    {
                        float x = i * h + h2;
                        float y = j * h;
                        float z = k * h + h2;
                        float u = avg_u(i, j, k);
                        float v = m_cells[i][j][k].v;
                        float w = m_cells[i][j][k].w;

                        x = x - p_delta_time * u;
                        y = y - p_delta_time * v;
                        z = z - p_delta_time * w;
                        v = sample_field(x, y, z, 1);

                        m_cells[i][j][k].new_v = v;
                    }

                    if (m_cells[i][j][k].s != 0.0f && m_cells[i][j][k - 1].s && i < m_size_x - 1 && j < m_size_y - 1)
                    {
                        float x = i * h + h2;
                        float y = j * h + h2;
                        float z = k * h;
                        float u = m_cells[i][j][k].u;
                        float v = m_cells[i][j][k].v;
                        float w = avg_w(i, j, k);

                        x = x - p_delta_time * u;
                        y = y - p_delta_time * v;
                        z = z - p_delta_time * w;
                        w = sample_field(x, y, z, 2);

                        m_cells[i][j][k].new_w = w;
                    }
                }
            }
        }
    }

    void FluidSimulation::draw(Camera *p_camera, Light *p_light)
    {
        update_physics(0.016f);
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
        // TODO
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
                    if (m_cells[x][y][z].s == 1.0f)
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

    float FluidSimulation::avg_u(uint16_t p_i, uint16_t p_j, uint16_t p_k)
    {
        // Average u velocity at the center of a cell
        return 0.25f * (m_cells[p_i][p_j][p_k].u +
                        m_cells[p_i + 1][p_j][p_k].u +
                        m_cells[p_i][p_j][p_k + 1].u +
                        m_cells[p_i + 1][p_j][p_k + 1].u);
    }

    float FluidSimulation::avg_v(uint16_t p_i, uint16_t p_j, uint16_t p_k)
    {
        // Average v velocity at the center of a cell
        return 0.25f * (m_cells[p_i][p_j][p_k].v +
                        m_cells[p_i][p_j + 1][p_k].v +
                        m_cells[p_i][p_j][p_k + 1].v +
                        m_cells[p_i][p_j + 1][p_k + 1].v);
    }

    float FluidSimulation::avg_w(uint16_t p_i, uint16_t p_j, uint16_t p_k)
    {
        // Average w velocity at the center of a cell
        return 0.25f * (m_cells[p_i][p_j][p_k].w +
                        m_cells[p_i + 1][p_j][p_k].w +
                        m_cells[p_i][p_j + 1][p_k].w +
                        m_cells[p_i + 1][p_j + 1][p_k].w);
    }

    float FluidSimulation::sample_field(float p_x, float p_y, float p_z, uint8_t p_field)
    {
        // float n = m_size_y;
        float h = m_grid_size;
        float h2 = 0.5f * h;
        float h1 = 1.0f / h;

        float x = fmax(h, fmin(p_x, m_size_x * h));
        float y = fmax(h, fmin(p_y, m_size_y * h));
        float z = fmax(h, fmin(p_z, m_size_z * h));

        float dx = 0.0f;
        float dy = 0.0f;
        float dz = 0.0f;

        switch (p_field)
        {
        case 0: // U FIELD
            dy = h2;
            dz = h2;
            break;
        case 1: // V FIELD
            dx = h2;
            dz = h2;
            break;
        case 2: // W FIELD
            dx = h2;
            dy = h2;
            break;
        case 3: // S FIELD
            dx = h2;
            dy = h2;
            dz = h2;
            break;
        default:
            break;
        }

        uint16_t x0 = fmax(m_size_x - 1, floor((x - dx) * h1));
        float tx = ((x - dx) - x0 * h) * h1;
        uint16_t x1 = fmin(x0 + 1, m_size_x - 1);

        uint16_t y0 = fmax(m_size_y - 1, floor((y - dy) * h1));
        float ty = ((y - dy) - y0 * h) * h1;
        uint16_t y1 = fmin(y0 + 1, m_size_y - 1);

        uint16_t z0 = fmax(m_size_z - 1, floor((z - dz) * h1));
        float tz = ((z - dz) - z0 * h) * h1;
        uint16_t z1 = fmin(z0 + 1, m_size_z - 1);

        float sx = 1.0f - tx;
        float sy = 1.0f - ty;
        float sz = 1.0f - tz;

        float val = 0.0f;
        switch (p_field)
        {
        case 0: // U FIELD
            val = sx * sy * sz * m_cells[x0][y0][z0].u 
            + tx * sy * sz * m_cells[x1][y0][z0].u 
            + sx * ty * sz * m_cells[x0][y1][z0].u 
            + tx * ty * sz * m_cells[x1][y1][z0].u 
            + sx * sy * tz * m_cells[x0][y0][z1].u 
            + tx * sy * tz * m_cells[x1][y0][z1].u 
            + sx * ty * tz * m_cells[x0][y1][z1].u 
            + tx * ty * tz * m_cells[x1][y1][z1].u;
            break;
        case 1: // V FIELD
            val = sx * sy * sz * m_cells[x0][y0][z0].v
            + tx * sy * sz * m_cells[x1][y0][z0].v
            + sx * ty * sz * m_cells[x0][y1][z0].v
            + tx * ty * sz * m_cells[x1][y1][z0].v
            + sx * sy * tz * m_cells[x0][y0][z1].v
            + tx * sy * tz * m_cells[x1][y0][z1].v
            + sx * ty * tz * m_cells[x0][y1][z1].v
            + tx * ty * tz * m_cells[x1][y1][z1].v;
            break;
        case 2: // W FIELD
            val = sx * sy * sz * m_cells[x0][y0][z0].w
            + tx * sy * sz * m_cells[x1][y0][z0].w
            + sx * ty * sz * m_cells[x0][y1][z0].w
            + tx * ty * sz * m_cells[x1][y1][z0].w
            + sx * sy * tz * m_cells[x0][y0][z1].w
            + tx * sy * tz * m_cells[x1][y0][z1].w
            + sx * ty * tz * m_cells[x0][y1][z1].w
            + tx * ty * tz * m_cells[x1][y1][z1].w;
            break;
        case 3: // S FIELD
            val = sx * sy * sz * m_cells[x0][y0][z0].s
            + tx * sy * sz * m_cells[x1][y0][z0].s
            + sx * ty * sz * m_cells[x0][y1][z0].s
            + tx * ty * sz * m_cells[x1][y1][z0].s
            + sx * sy * tz * m_cells[x0][y0][z1].s
            + tx * sy * tz * m_cells[x1][y0][z1].s
            + sx * ty * tz * m_cells[x0][y1][z1].s
            + tx * ty * tz * m_cells[x1][y1][z1].s;
            break;
        default:

            break;
        }
        return val;
    }
}

#include "objects/fluid_simulation_2D.h"

namespace FAVE
{
    FluidSimulation2D::FluidSimulation2D(Material &p_material, float p_fluid_density, float p_grid_size, uint16_t p_size_x, uint16_t p_size_y, uint16_t p_water_level)
        : FluidSimulation(p_material, p_fluid_density, m_grid_size), m_size_x(p_size_x), m_size_y(p_size_y), m_water_level(p_water_level)
    {
        m_cells = new GridCell *[m_size_x];
        for (uint16_t x = 0; x < m_size_x; ++x)
        {
            m_cells[x] = new GridCell[m_size_y];
            for (uint16_t y = 0; y < m_size_y; ++y)
            {
                m_cells[x][y].p = 0.0f;

                m_cells[x][y].u = 0.0f;
                m_cells[x][y].v = 0.0f;
                m_cells[x][y].w = 0.0f;
                m_cells[x][y].new_u = m_cells[x][y].u;
                m_cells[x][y].new_v = m_cells[x][y].v;
                m_cells[x][y].new_w = m_cells[x][y].w;

                m_cells[x][y].s = (y <= m_water_level) ? 1.0f : 0.0f;
                m_cells[x][y].new_s = m_cells[x][y].s;

                if (x == 0 || x == m_size_x - 1 || y == 0 || y == m_size_y - 1)
                    m_cells[x][y].s = 0.0f; // TODO this is not solid, this is air
            }
        }
    }

    void FluidSimulation2D::fixedUpdate(float p_fixed_delta_time)
    {
        float time_step = find_time_step(p_fixed_delta_time);
        for (uint16_t i = 0; i < m_solver_steps; ++i)
        {
            solve_incompresabillity(p_fixed_delta_time);
            advect(p_fixed_delta_time);
        }
    }

    void FluidSimulation2D::update(float p_delta_time)
    {
        recognise_geometry();
    }

    void FluidSimulation2D::solve_incompresabillity(float p_fixed_delta_time)
    {
        float dt = find_time_step(p_fixed_delta_time);
        float cp = m_fluid_density * m_grid_size / dt;

        for (uint8_t iter = 0; iter < m_solver_steps; iter++)
        {
            for (uint16_t i = 1; i < m_size_x - 1; i++)
            {
                for (uint16_t j = 1; j < m_size_y - 1; j++)
                {
                    if (m_cells[i][j].s == 0.0f)
                    {
                        m_cells[i][j].p = 0.0f;
                        continue;
                    }

                    float sx0 = m_cells[i - 1][j].s;
                    float sx1 = m_cells[i + 1][j].s;
                    float sy0 = m_cells[i][j - 1].s;
                    float sy1 = m_cells[i][j + 1].s;
                    float s = sx0 + sx1 + sy0 + sy1;
                    if (s == 0.0f)
                    {
                        // m_cells[i][j].p = 0.0f;
                        continue;
                    }

                    float div = (m_cells[i + 1][j].u - m_cells[i][j].u) + (m_cells[i][j + 1].v - m_cells[i][j].v);
                    float p = -div / s;
                    p *= m_over_relaxation;

                    m_cells[i][j].p = p;

                    m_cells[i][j].u -= sx0 * p;
                    m_cells[i + 1][j].u += sx1 * p;
                    m_cells[i][j].v -= sy0 * p;
                    m_cells[i][j + 1].v += sy1 * p;
                }
            }
        }
    }

    void FluidSimulation2D::extrapolate_velocity()
    {
        for (uint16_t i = 0; i < m_size_x; i++)
        {
            m_cells[i][0].u = m_cells[i][1].u;
            m_cells[i][m_size_y - 1].v = m_cells[i][m_size_y - 2].v;
        }

        for (uint16_t j = 0; j < m_size_y; j++)
        {
            m_cells[0][j].u = m_cells[1][j].u;
            m_cells[m_size_x - 1][j].v = m_cells[m_size_x - 2][j].v;
        }
    }

    void FluidSimulation2D::advect(float p_fixed_delta_time)
    {
        float dt = find_time_step(p_fixed_delta_time);

        for (uint16_t i = 1; i < m_size_x - 1; i++)
        {
            for (uint16_t j = 1; j < m_size_y - 1; j++)
            {
                if (m_cells[i][j].s == 0.0f)
                {
                    m_cells[i][j].new_u = 0.0f;
                    m_cells[i][j].new_v = 0.0f;
                    continue;
                }

                float u = m_cells[i][j].u;
                float v = m_cells[i][j].v;

                float x = i - u * dt;
                float y = j - v * dt;

                if (x < 0.5f)
                    x = 0.5f;
                if (x > m_size_x - 1.5f)
                    x = m_size_x - 1.5f;
                if (y < 0.5f)
                    y = 0.5f;
                if (y > m_size_y - 1.5f)
                    y = m_size_y - 1.5f;

                m_cells[i][j].new_u = sample_field(x, y, FieldType_t::U_field);
                m_cells[i][j].new_v = sample_field(x, y, FieldType_t::V_field);
            }
        }

        for (uint16_t i = 1; i < m_size_x - 1; i++)
        {
            for (uint16_t j = 1; j < m_size_y - 1; j++)
            {
                m_cells[i][j].u = m_cells[i][j].new_u;
                m_cells[i][j].v = m_cells[i][j].new_v;
            }
        }

        extrapolate_velocity();
    }

    float FluidSimulation2D::sample_field(float p_x, float p_y, FieldType_t p_field)
    {
        uint16_t x = (uint16_t)p_x;
        uint16_t y = (uint16_t)p_y;
        float fx = p_x - x;
        float fy = p_y - y;

        switch (p_field)
        {
        case FieldType_t::U_field:
            return (1.0f - fx) * (1.0f - fy) * m_cells[x][y].u +
                   fx * (1.0f - fy) * m_cells[x + 1][y].u +
                   (1.0f - fx) * fy * m_cells[x][y + 1].u +
                   fx * fy * m_cells[x + 1][y + 1].u;
        case FieldType_t::V_field:
            return (1.0f - fx) * (1.0f - fy) * m_cells[x][y].v +
                   fx * (1.0f - fy) * m_cells[x + 1][y].v +
                   (1.0f - fx) * fy * m_cells[x][y + 1].v +
                   fx * fy * m_cells[x + 1][y + 1].v;
        case FieldType_t::W_field:
            return (1.0f - fx) * (1.0f - fy) * m_cells[x][y].w +
                   fx * (1.0f - fy) * m_cells[x + 1][y].w +
                   (1.0f - fx) * fy * m_cells[x][y + 1].w +
                   fx * fy * m_cells[x + 1][y + 1].w;
        case FieldType_t::S_field:
            return (1.0f - fx) * (1.0f - fy) * m_cells[x][y].s +
                   fx * (1.0f - fy) * m_cells[x + 1][y].s +
                   (1.0f - fx) * fy * m_cells[x][y + 1].s +
                   fx * fy * m_cells[x + 1][y + 1].s;
        default:
            return 0.0f;
        }
    }

    float FluidSimulation2D::avg_u(uint16_t p_i, uint16_t p_j)
    {
        return 0.25f * (m_cells[p_i][p_j].u + m_cells[p_i + 1][p_j].u + m_cells[p_i][p_j + 1].u + m_cells[p_i + 1][p_j + 1].u);
    }

    float FluidSimulation2D::avg_v(uint16_t p_i, uint16_t p_j)
    {
        return 0.25f * (m_cells[p_i][p_j].v + m_cells[p_i + 1][p_j].v + m_cells[p_i][p_j + 1].v + m_cells[p_i + 1][p_j + 1].v);
    }

    float FluidSimulation2D::find_time_step(float p_fixed_delta_time)
    {
        float max_vel = 0.0f;
        for (uint16_t x = 1; x < m_size_x - 1; ++x)
        {
            for (uint16_t y = 1; y < m_size_y - 1; ++y)
            {
                float vel = glm::length(glm::vec3(m_cells[x][y].u, m_cells[x][y].v, m_cells[x][y].w));
                if (vel > max_vel)
                    max_vel = vel;
            }
        }

        float time_step = m_grid_size / max_vel;
        return time_step;
    }

    void FluidSimulation2D::recognise_geometry()
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
                glm::vec3 cubePos = glm::vec3(x, y, 0) * m_grid_size;

                for (const auto &vert : cubeVertices)
                {
                    Vertex vertex;
                    vertex.position = cubePos + vert * m_grid_size;

                    // Adjust color based on velocity
                    float speed = abs(glm::length(glm::vec3(m_cells[x][y].u, m_cells[x][y].v, m_cells[x][y].w)));
                    vertex.color = glm::vec3(0.3f * speed + 0.1f, 0.3f * speed + 0.1f, 0.7f * speed + 0.1f);

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

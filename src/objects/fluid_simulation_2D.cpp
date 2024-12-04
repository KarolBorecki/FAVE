#include "objects/fluid_simulation_2D.h"

// #define OBSERVE
uint16_t observed_x = 60;
uint16_t observed_y = 50;

namespace FAVE
{
    FluidSimulation2D::FluidSimulation2D(Material &p_material, float p_fluid_density, float p_grid_size, uint16_t p_size_x, uint16_t p_size_y, uint16_t p_water_level)
        : FluidSimulation::FluidSimulation(p_material, p_fluid_density, p_grid_size), m_size_x(p_size_x), m_size_y(p_size_y), m_water_level(p_water_level)
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

                if (x == 0 || x == m_size_x - 1 || y == 0 || y == m_size_y - 1)
                    m_cells[x][y].s = 0.0f; // TODO this is not solid, this is air
                else if (y <= m_water_level)
                    m_cells[x][y].s = 1.0f;
                else
                    m_cells[x][y].s = 0.0f;
                m_cells[x][y].new_s = m_cells[x][y].s;

                if (x > 5 && x < m_size_x - 5 && y == m_water_level)
                    m_cells[x][y].v = -10.0f;
            }
        }
    }

    void FluidSimulation2D::fixedUpdate(float p_fixed_delta_time)
    {
        float dt = find_time_step(p_fixed_delta_time);
        for (uint16_t i = 0; i < m_solver_steps; ++i)
        {
            for (uint16_t x = 1; x < m_size_x - 1; ++x)
            {
                for (uint16_t y = 1; y < m_size_y - 1; ++y)
                {
                    m_cells[x][y].p = 0.0f;
                    if (m_cells[x][y].s != 0.0 && m_cells[x][y - 1].s != 0.0)
                        m_cells[x][y].v += dt * GRAVITY;
                }
            }

            solve_incompresabillity(dt);
            advect(dt);
            extrapolate_velocity();
        }
    }

    void FluidSimulation2D::update(float p_delta_time)
    {
        recognise_geometry();
    }

    void FluidSimulation2D::applyForce(glm::vec2 p_force, glm::vec2 p_position, float p_radius)
    {
        uint16_t x0 = fmax(0, floor((p_position.x - p_radius)));
        uint16_t x1 = fmin(m_size_x - 1, ceil((p_position.x + p_radius)));
        uint16_t y0 = fmax(0, floor((p_position.y - p_radius)));
        uint16_t y1 = fmin(m_size_y - 1, ceil((p_position.y + p_radius)));

        for (uint16_t x = x0; x <= x1; ++x)
        {
            for (uint16_t y = y0; y <= y1; ++y)
            {
                if (m_cells[x][y].s == 0.0f)
                    continue;
                m_cells[x][y].u += p_force.x;
                m_cells[x][y].u += p_force.y;

                log("applied force to cella[%d][%d] u = %.2f v = %.2f s = %.2f", x, y, m_cells[x][y].u, m_cells[x][y].v, m_cells[x][y].s);
            }
        }
    }

    void
    FluidSimulation2D::solve_incompresabillity(float p_dt)
    {
        float cp = m_fluid_density * m_grid_size / p_dt;

        for (uint8_t iter = 0; iter < m_solver_steps; iter++)
        {
            for (uint16_t i = 1; i < m_size_x - 1; i++)
            {
                for (uint16_t j = 1; j < m_size_y - 1; j++)
                {
                    if (m_cells[i][j].s == 0.0f)
                        continue;

                    float sx0 = m_cells[i - 1][j].s;
                    float sx1 = m_cells[i + 1][j].s;
                    float sy0 = m_cells[i][j - 1].s;
                    float sy1 = m_cells[i][j + 1].s;
                    float s = sx0 + sx1 + sy0 + sy1;
                    if (s == 0.0f)
                        continue;

                    float div = (m_cells[i + 1][j].u - m_cells[i][j].u) + (m_cells[i][j + 1].v - m_cells[i][j].v);
                    float p = -div / s;
                    p *= m_over_relaxation;
#ifdef OBSERVE
                    if (i == observed_x && j == observed_y)
                    {
                        log("cell[%d][%d] u = %.2f v = %.2f p = %.2f     pLoc = %.2f div = %.2f s = %.2f", observed_x, observed_y, m_cells[observed_x][observed_y].u, m_cells[observed_x][observed_y].v, m_cells[observed_x][observed_y].p, p, div, s);
                        log("S:");
                        log("     %.2f     ", sy1);
                        log("%.2f %.2f %.2f", sx0, m_cells[i][j].s, sx1);
                        log("     %.2f     ", sy0);
                        log("speeeeed:");
                        log("     %.2f     ", m_cells[i][j].v);
                        log("%.2f      %.2f", m_cells[i][j].u, m_cells[i + 1][j].u);
                        log("     %.2f     ", m_cells[i][j - 1].v);
                    }
#endif
                    m_cells[i][j].p += p * cp;

                    m_cells[i][j].u -= sx0 * p;
                    m_cells[i + 1][j].u += sx1 * p;
                    m_cells[i][j].v -= sy0 * p;
                    m_cells[i][j + 1].v += sy1 * p;
                }
            }
        }
#ifdef OBSERVE
        log("--------------------");
#endif
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

    void FluidSimulation2D::advect(float p_dt)
    {
        float h = m_grid_size;
        float h2 = 0.5f * h;

        for (uint16_t i = 1; i < m_size_x; i++)
        {
            for (uint16_t j = 1; j < m_size_y; j++)
            {
                m_cells[i][j].new_u = m_cells[i][j].u;
                m_cells[i][j].new_v = m_cells[i][j].v;

                if (m_cells[i][j].s == 0.0f && m_cells[i - 1][j].s == 0.0f)
                {
                    float x = i * h;
                    float y = j * h + h2;
                    float u = m_cells[i][j].u;
                    float v = avg_v(i, j);

                    x = x - p_dt * u;
                    y = y - p_dt * v;
                    u = sample_field(x, y, FieldType_t::U_field);

                    m_cells[i][j].new_u = u;
                }

                if (m_cells[i][j].s == 0.0f && m_cells[i][j - 1].s == 0.0f)
                {
                    float x = i * h + h2;
                    float y = j * h;
                    float u = avg_u(i, j);
                    float v = m_cells[i][j].v;

                    x = x - p_dt * u;
                    y = y - p_dt * v;
                    u = sample_field(x, y, FieldType_t::V_field);

                    m_cells[i][j].new_v = v;
                }
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

        // actually I advect s here too
        for (uint16_t i = 1; i < m_size_x - 1; i++)
        {
            for (uint16_t j = 1; j < m_size_y - 1; j++)
            {
                if (m_cells[i][j].s != 0.0f)
                {
                    float u = (m_cells[i][j].u + m_cells[i + 1][j].u) * 0.5f;
                    float v = (m_cells[i][j].v + m_cells[i + 1][j].v) * 0.5f;

                    float x = i * h + h2 - p_dt * u;
                    float y = j * h + h2 - p_dt * v;
                    float m = sample_field(x, y, FieldType_t::S_field);

                    m_cells[i][j].new_m = m;
                }
            }
        }

        for (uint16_t i = 1; i < m_size_x - 1; i++)
        {
            for (uint16_t j = 1; j < m_size_y - 1; j++)
            {
                m_cells[i][j].m = m_cells[i][j].new_m;
            }
        }
#ifdef OBSERVE
        log("cell[%d][%d] u = %.2f v = %.2f s = %.2f", observed_x, observed_y, m_cells[observed_x][observed_y].u, m_cells[observed_x][observed_y].v, m_cells[observed_x][observed_y].s);
#endif
    }

    float FluidSimulation2D::sample_field(float p_x, float p_y, FieldType_t p_field)
    {
        float h = m_grid_size;
        float h2 = 0.5f * h;
        float h1 = 1.0f / h;

        float x = fmax(h, fmin(p_x, m_size_x * h));
        float y = fmax(h, fmin(p_y, m_size_y * h));

        float dx = 0.0f;
        float dy = 0.0f;

        switch (p_field)
        {
        case FieldType_t::U_field:
            dy = h2;
            break;
        case FieldType_t::V_field:
            dx = h2;
            break;
        case FieldType_t::W_field:
            break;
        case FieldType_t::S_field:
            dx = h2;
            dy = h2;
        default:
            return 0.0f;
        }

        uint16_t x0 = fmin(m_size_x - 1, floor((x - dx) * h1));
        float tx = ((x - dx) - x0 * h) * h1;
        uint16_t x1 = fmin(x0 + 1, m_size_x - 1);

        uint16_t y0 = fmin(m_size_y - 1, floor((y - dy) * h1));
        float ty = ((y - dy) - y0 * h) * h1;
        uint16_t y1 = fmin(y0 + 1, m_size_y - 1);

        float sx = 1.0f - tx;
        float sy = 1.0f - ty;

        switch (p_field)
        {
        case FieldType_t::U_field:
            return sx * sy * m_cells[x0][y0].u + tx * sy * m_cells[x1][y0].u + sx * ty * m_cells[x0][y1].u + tx * ty * m_cells[x1][y1].u;
        case FieldType_t::V_field:
            return sx * sy * m_cells[x0][y0].v + tx * sy * m_cells[x1][y0].v + sx * ty * m_cells[x0][y1].v + tx * ty * m_cells[x1][y1].v;
        case FieldType_t::W_field:
            return 0.0f;
        case FieldType_t::S_field:
            return sx * sy * m_cells[x0][y0].s + tx * sy * m_cells[x1][y0].s + sx * ty * m_cells[x0][y1].s + tx * ty * m_cells[x1][y1].s;
        default:
            return 0.0f;
        }
    }

    float FluidSimulation2D::avg_u(uint16_t p_i, uint16_t p_j)
    {
        return 0.25f * (m_cells[p_i][p_j].u + m_cells[p_i - 1][p_j].u + m_cells[p_i][p_j - 1].u + m_cells[p_i - 1][p_j - 1].u);
    }

    float FluidSimulation2D::avg_v(uint16_t p_i, uint16_t p_j)
    {
        return 0.25f * (m_cells[p_i][p_j].v + m_cells[p_i - 1][p_j].v + m_cells[p_i][p_j - 1].v + m_cells[p_i - 1][p_j - 1].v);
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

    std::array<float, 3> FluidSimulation2D::getSciColor(float val, float minVal, float maxVal)
    {
        val = fmin(fmax(val, minVal), maxVal - 0.0001);
        float d = maxVal - minVal;
        val = d == 0.0 ? 0.5 : (val - minVal) / d;
        float m = 0.25;
        uint8_t num = floor(val / m);
        float s = (val - num * m) / m;
        float r, g, b;

        switch (num)
        {
        case 0:
            r = 0.0;
            g = s;
            b = 1.0;
            break;
        case 1:
            r = 0.0;
            g = 1.0;
            b = 1.0 - s;
            break;
        case 2:
            r = s;
            g = 1.0;
            b = 0.0;
            break;
        case 3:
            r = 1.0;
            g = 1.0 - s;
            b = 0.0;
            break;
        }

        return {1.0f * r, 1.0f * g, 1.0f * b};
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
        float min_p = m_cells[0][0].p;
        float max_p = m_cells[0][0].p;
        for (uint16_t x = 0; x < m_size_x; ++x)
        {
            for (uint16_t y = 0; y < m_size_y; ++y)
            {
                if (m_cells[x][y].p < min_p)
                    min_p = m_cells[x][y].p;
                if (m_cells[x][y].p > max_p)
                    max_p = m_cells[x][y].p;
            }
        }

        for (uint16_t x = 0; x < m_size_x; ++x)
        {
            for (uint16_t y = 0; y < m_size_y; ++y)
            {
                if (m_cells[x][y].s == 0.0f)
                    continue;
                glm::vec3 cubePos = glm::vec3(x, y, 0) * m_grid_size;
                for (const auto &vert : cubeVertices)
                {
                    Vertex vertex;
                    vertex.position = cubePos + vert * m_grid_size;

                    std::array<float, 3> color = getSciColor(m_cells[x][y].p, min_p, max_p);
                    vertex.color = glm::vec3(color[0], color[1], color[2]);

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

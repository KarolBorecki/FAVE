#include "objects/fluid_simulation_2D.h"

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
                    m_cells[x][y].s = 0.0f;
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

        m_current_min_pressure = FLT_MAX;
        m_current_max_pressure = FLT_MIN;
        float pressure_sum = 0.0f;
        for (uint16_t x = 0; x < m_size_x; ++x)
        {
            for (uint16_t y = 0; y < m_size_y; ++y)
            {
                if (m_cells[x][y].p < m_current_min_pressure)
                    m_current_min_pressure = m_cells[x][y].p;
                if (m_cells[x][y].p > m_current_max_pressure)
                    m_current_max_pressure = m_cells[x][y].p;

                pressure_sum += m_cells[x][y].p;
            }
        }
        m_current_avg_pressure = pressure_sum / (m_size_x * m_size_y);

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

            solve_incompresabillity();
            advect();
            extrapolate_velocity();
        }
    }

    void FluidSimulation2D::update(float p_delta_time)
    {
        recognise_geometry();
    }

    

    float FluidSimulation2D::find_time_step(float p_fixed_delta_time)
    {
        const float CFL = 0.5f;
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

        m_current_dt = CFL * (m_grid_size / max_vel);
        m_current_dt = p_fixed_delta_time;
        m_current_dt = (m_grid_size / max_vel);

        return m_current_dt;
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

        for (uint16_t x = 0; x < m_size_x; ++x)
        {
            for (uint16_t y = 0; y < m_size_y; ++y)
            {
                glm::vec3 cubePos = glm::vec3(x, y, 0) * m_grid_size;
                for (const auto &vert : cubeVertices)
                {
                    Vertex vertex;
                    vertex.position = cubePos + vert * m_grid_size;

                    std::array<float, 3> color = getSciColor(m_cells[x][y].p, m_current_min_pressure, m_current_max_pressure);
                    if (m_cells[x][y].s == 0.0f)
                    {
                        color = {0.1f, 0.1f, 0.1f};
                    }
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

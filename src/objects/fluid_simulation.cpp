#include "objects/fluid_simulation.h"

namespace FAVE
{
    FluidSimulation(uint16_t p_size_x, uint16_t p_size_y, uint16_t p_size_z, uint16_t p_water_level, float p_cube_size) : m_size_x(p_size_x), m_size_y(p_size_y), m_size_z(p_size_z), m_water_level(p_water_level), m_cube_size(p_cube_size)
    {
        m_cubes = new bool **[m_width];
        for (uint16_t x = 0; x < m_size_x; x++)
        {
            m_cubes[x] = new bool *[m_height];
            for (uint16_t y = 0; y < m_size_y; y++)
            {
                m_cubes[x][y] = new bool[m_size_z];
                for (uint16_t z = 0; z < m_size_z; z++)
                {
                    m_cubes[x][y][z] = y <= m_water_level;
                }
            }
        }
    }

    ~FluidSimulation()
    {
        destroy();
    }

    void FluidSimulation::draw(Camera *p_camera, Light *p_light)
    {
        for (Cube cube : m_cubes)
        {
            // Draw the cube
        }
        VAO VAO1;
        VAO1.Bind();

        // Generates Vertex Buffer Object and links it to vertices
        VBO VBO1(vertices, sizeof(vertices));
        // Generates Element Buffer Object and links it to indices
        EBO EBO1(indices, sizeof(indices));

        // Links VBO attributes such as coordinates and colors to VAO
        VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 8 * sizeof(float), (void *)0);
        VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        VAO1.LinkAttrib(VBO1, 2, 2, GL_FLOAT, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        // Unbind all to prevent accidentally modifying them
        VAO1.Unbind();
        VBO1.Unbind();
        EBO1.Unbind();
        glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(int), GL_UNSIGNED_INT, 0);
    }

    void FluidSimulation::destroy()
    {
        // Destroy the fluid simulation
    }

    void FluidSimulation::recognise_geometry()
    {
        uint16_t next_vertex_id = 0;
        for (uint16_t x = 0; x < m_size_x; x++)
        {
            for (uint16_t z = 0; z < m_size_z; z++)
            {
                for (uint16_t y = 0; !m_cubes[x][y][z]; y++)
                    y++; // Skip "water cube" body
                m_vericies[next_vertex_id++] = x * m_cube_size;
                m_vericies[next_vertex_id++] = y * m_cube_size;
                m_vericies[next_vertex_id++] = z * m_cube_size;
            }
        }
    }
#ifndef FAVE_FLUID_SIMULATION_H
#define FAVE_FLUID_SIMULATION_H

#include "objects/render_object.h"

namespace FAVE
{
    // Vertices coordinates
    GLfloat vertices[] =
        { //     COORDINATES     /        COLORS      /   TexCoord  //
            -0.5f, 0.0f, 0.5f, 0.83f, 0.70f, 0.44f, 0.0f, 0.0f,
            -0.5f, 0.0f, -0.5f, 0.83f, 0.70f, 0.44f, 5.0f, 0.0f,
            0.5f, 0.0f, -0.5f, 0.83f, 0.70f, 0.44f, 0.0f, 0.0f,
            0.5f, 0.0f, 0.5f, 0.83f, 0.70f, 0.44f, 5.0f, 0.0f,
            0.0f, 0.8f, 0.0f, 0.92f, 0.86f, 0.76f, 2.5f, 5.0f};

    // Indices for vertices order
    GLuint indices[] =
        {
            0, 1, 2,
            0, 2, 3,
            0, 1, 4,
            1, 2, 4,
            2, 3, 4,
            3, 0, 4};
    class FluidSimulation : public RenderObject
    {
    public:
        FluidSimulation(uint16_t p_width, uint16_t p_height, uint16_t p_water_level, float p_cube_size);
        virtual ~FluidSimulation();

        void draw(Camera *p_camera, Light *p_light);

        void destroy();

    private:
        uint16_t m_size_x;
        uint16_t m_size_y;
        uint16_t m_size_z;
        uint16_t m_water_level;

        float m_cube_size;

        bool ***m_cubes; // japierdole

        GLfloat m_vertices[1024];
        GLuint m_indices[1024];
        uint16_t m_verticies_count{0};

        void recognise_geometry();
    };
}

#endif // FAVE_FLUID_SIMULATION_H
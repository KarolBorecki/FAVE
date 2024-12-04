#ifndef FAVE_FLUID_SIMULATION_2D_H
#define FAVE_FLUID_SIMULATION_2D_H

#include <stdlib.h>
#include <cmath>
#include <time.h>
#include <chrono>
#include <array>
#include <vector>

#include <glm/glm.hpp>

#include "objects/fluid_simulation.h"
#include "buffers/vao.h"
#include "buffers/vbo.h"
#include "buffers/ebo.h"
#include "objects/camera.h"
#include "objects/light.h"
#include "objects/render_object.h"
#include "materials/material.h"

#define FAVE_2D_FLUID_SIMULATION

namespace FAVE
{
    class FluidSimulation2D : public FluidSimulation
    {
    public:
        FluidSimulation2D(Material &p_material, float p_fluid_density, float p_grid_size, uint16_t p_size_x, uint16_t p_size_y, uint16_t p_water_level);

        void update(float p_delta_time) override;
        void fixedUpdate(float p_fixed_delta_time) override;

        void applyForce(glm::vec2 p_force, glm::vec2 p_position, float p_radius) override;

    private:
        uint16_t m_size_x;
        uint16_t m_size_y;
        uint16_t m_water_level;

        GridCell_t **m_cells;

        uint16_t m_solver_steps = 16;
        float m_over_relaxation = 1.1f;

        float find_time_step(float p_fixed_delta_time);
        void solve_incompresabillity(float p_dt);
        void extrapolate_velocity();
        void advect(float p_dt);

        void recognise_geometry(); // TODO: Implement this method

        float sample_field(float p_x, float p_y, FieldType_t p_field);
        float avg_u(uint16_t p_i, uint16_t p_j);
        float avg_v(uint16_t p_i, uint16_t p_j);

        std::array<float, 3> getSciColor(float val, float minVal, float maxVal);
    };
}

#endif // FAVE_FLUID_SIMULATION_2D_H
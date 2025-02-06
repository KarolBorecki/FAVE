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
        float h = 0.0f;
        float h_inv = 0.0f;

        GridCell_t **m_cells;

        uint16_t m_solver_steps = 15;
        float m_over_relaxation = 1.9f;
        float m_flip_coefficient = 0.9f;

        float m_current_dt = 0.0f;
        float m_current_avg_pressure = 0.0f;
        float m_current_max_pressure = 0.0f;
        float m_current_min_pressure = 0.0f;


        // numerical calculation methods to solve fluid
        void integrate_particles();
        void push_particles_apart();
        void handle_particle_collision();
        void transfer_velocities();
        void update_particle_density();
        void solve_incompresabillity();

        // methods to visualize the fluid
        void recognise_geometry(); // TODO: Implement this method

        // methods utils
        float find_time_step(float p_fixed_delta_time);
        std::array<float, 3> getSciColor(float val, float minVal, float maxVal);
    };
}

#endif // FAVE_FLUID_SIMULATION_2D_H
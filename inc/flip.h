#ifndef FAVE_MAC_H
#define FAVE_MAC_H

#include <cstdio>
#include <cinttypes>

#include <math.h>

#include <glm/glm.hpp>

#include "definitions.h"
#include "types.h"
#include "utils.h"
#include "buffers/vbo.h"
#include "obstacle.h"

#ifdef __cplusplus
extern "C"
{
#endif

    enum CellType : int
    {
        FLUID = 0,
        SOLID = 1,
        AIR = 2
    };
    
    typedef struct MacGrid
    {
        float density;
        int f_num_x;
        int f_num_y;
        float h;
        float f_inv_spacing;
        int f_num_cells;

        float *u;            // of size f_num_cells
        float *v;            // of size f_num_cells
        float *du;           // of size f_num_cells
        float *dv;           // of size f_num_cells
        float *prev_u;       // of size f_num_cells
        float *prev_v;       // of size f_num_cells
        float *p;            // of size f_num_cells
        float *s;            // of size f_num_cells
        CellType *cell_type; // of size f_num_cells
        float *cell_color;   // of size f_num_cells * 3

        int max_particles;
        float *particle_pos;     // of size max_particles * 2
        float *particle_vel;     // of size max_particles * 2
        float *particle_density; // of size f_num_cells
        float particle_rest_density;

        float particle_radius;
        float p_inv_spacing;
        int p_num_x;
        int p_num_y;
        int p_num_cells; // p_num_x * p_num_y

        int *num_cell_particles;  // of size p_num_cells
        int *first_cell_particle; // of size p_num_cells + 1
        int *cell_particle_ids;   // of size max_particles

        int num_particles;

    } MacGrid_t;

    void MAC_init(MacGrid_t *grid, float density, float size_x, float size_y, float cell_size, float marker_count, int marker_size);
    void MAC_handleObstacle(MacGrid_t *grid, Obstacle_t *obstacle, float dt);
    void MAC_integrateParticles(MacGrid_t *grid, float dt, float gravity);
    void MAC_pushParticlesApart(MacGrid_t *grid, int numIters, float dt);
    void MAC_transferVelocities(MacGrid_t *grid, int toGrid, float flipRatio);
    void MAC_updateParticleDensity(MacGrid_t *grid);
    void MAC_solveIncompressibility(MacGrid_t *grid, int numIters, float dt, float overRelaxation);
    Pair_t MAC_transformGridToVerticies(MacGrid_t *grid, Vertex_t *vertices, GLuint *indices, int show_sci);
    Pair_t MAC_transformMarkersToVertices(MacGrid_t *grid, Vertex_t *markerVertices, GLuint *markerIndices);
    void MAC_destroy(MacGrid_t *grid);

#ifdef __cplusplus
}
#endif

#endif // FAVE_MAC_H
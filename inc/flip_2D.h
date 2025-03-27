#ifndef FAVE_FLIP2D_H
#define FAVE_FLIP2D_H

#include <cstdio>
#include <cinttypes>

#include <math.h>

#include <cglm/cglm.h>
#include <cglm/struct.h>

#include "definitions.h"
#include "types.h"
#include "utils.h"
#include "buffers/vbo.h"
#include "obstacle.h"
#include "marching_cubes.h"

#ifdef __cplusplus
extern "C"
{
#endif
    void FLIP2D_init(FlipGrid_t *grid, float density, float size_x, float size_y, float cell_size, float marker_count, int marker_size);
    void FLIP2D_handleObstacle(FlipGrid_t *grid, Obstacle_t *obstacle, float dt);
    void FLIP2D_integrateParticles(FlipGrid_t *grid, float dt, float gravity);
    void FLIP2D_pushParticlesApart(FlipGrid_t *grid, int numIters, float dt);
    void FLIP2D_transferVelocities(FlipGrid_t *grid, int toGrid, float flipRatio);
    void FLIP2D_updateParticleDensity(FlipGrid_t *grid);
    void FLIP2D_solveIncompressibility(FlipGrid_t *grid, int numIters, float dt, float overRelaxation);
    Pair_t FLIP2D_transformGridToVerticiesMarchingSquares(FlipGrid_t *grid, Vertex_t *vertices, GLuint *indices, int show_sci, int show_air, int show_solids);
#ifdef __cplusplus
}
#endif

#endif // FAVE_FLIP2D_H
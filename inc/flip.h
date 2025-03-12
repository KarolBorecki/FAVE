#ifndef FAVE_FLIP_H
#define FAVE_FLIP_H

#include <cstdio>
#include <cinttypes>

#include <math.h>

#include <glm/glm.hpp>

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
    void FLIP_init(FlipGrid_t *grid, float density, float size_x, float size_y, float size_z, float cell_size, float marker_count, int marker_size);
    void FLIP_handleObstacle(FlipGrid_t *grid, Obstacle_t *obstacle, float dt);
    void FLIP_integrateParticles(FlipGrid_t *grid, float dt, float gravity);
    void FLIP_pushParticlesApart(FlipGrid_t *grid, int numIters, float dt);
    void FLIP_transferVelocities(FlipGrid_t *grid, int toGrid, float flipRatio);
    void FLIP_updateParticleDensity(FlipGrid_t *grid);
    void FLIP_solveIncompressibility(FlipGrid_t *grid, int numIters, float dt, float overRelaxation);
    Pair_t FLIP_transformGridToVerticies(FlipGrid_t *grid, Vertex_t *vertices, GLuint *indices, int show_sci);
    Pair_t FLIP_transformGridToVerticiesMarchingCubes(FlipGrid_t *grid, Vertex_t *vertices, GLuint *indices, int show_sci);
    Pair_t FLIP_transformMarkersToVertices(FlipGrid_t *grid, Vertex_t *markerVertices, GLuint *markerIndices);
    void FLIP_destroy(FlipGrid_t *grid);

#ifdef __cplusplus
}
#endif

#endif // FAVE_FLIP_H
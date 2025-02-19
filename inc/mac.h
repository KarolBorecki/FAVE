#ifndef FAVE_MAC_H
#define FAVE_MAC_H

#include <cstdio>
#include <math.h>

#include <glm/glm.hpp>

#include "definitions.h"
#include "types.h"
#include "utils.h"
#include "buffers/vbo.h"
#include "obstacle.h"

// TODO remove
#define uint16_t int
#define uint8_t int

enum CellType : uint8_t
{
    FLUID = 0,
    SOLID = 1,
    AIR = 2
};

typedef struct GridPos
{
    uint16_t x;
    uint16_t y;
} GridPos_t;

typedef struct GridCell
{
    float p;
    float v, u, w; // v - up/down vec, u - left/right vec, w - forward/backward vec
    float dv, du, dw;
    float prevv, prevu, prevw;
    float s;
    float density; // particleDensity
    CellType type;
} GridCell_t;

typedef struct Marker
{
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec4 color;
} Marker_t;

typedef struct MacGrid
{
    float density;
    uint16_t size_x;
    uint16_t size_y;
    uint16_t total_size; // pNumCells

    uint16_t num_markers; // maxParticles

    GridCell_t *cells;
    Marker_t *markers;
    uint16_t *num_cell_markers;  // numCellParticles
    uint16_t *first_cell_marker; // firstCellParticle
    uint16_t *cell_marker_ids;   // cellParticleIds

    float cell_size;     // h
    float inv_cell_size; // fInvSpacing

    float marker_radius;      // particleRadius
    float marker_inv_spacing; // pInvSpacing

    float markers_rest_density; // particleRestDensity

} MacGrid_t;

void MAC_init(MacGrid_t *grid, float density, int size_x, int size_y, float cell_size);
void MAC_handleObstacle(MacGrid_t *grid, Obstacle_t *obstacle, float dt);
void MAC_integrateParticles(MacGrid_t *grid, float dt, float gravity);
void MAC_pushParticlesApart(MacGrid_t *grid, int numIters, float dt);
void MAC_transferVelocities(MacGrid_t *grid, int toGrid, float flipRatio);
void MAC_updateParticleDensity(MacGrid_t *grid);
void MAC_solveIncompressibility(MacGrid_t *grid, int numIters, float dt, float overRelaxation);
Pair_t MAC_transformGridToVerticies(MacGrid_t *grid, Vertex_t *vertices, GLuint *indices);
Pair_t MAC_transformMarkersToVertices(MacGrid_t *grid, Vertex_t *markerVertices, GLuint *markerIndices);
void MAC_destroy(MacGrid_t *grid);

#endif // FAVE_MAC_H
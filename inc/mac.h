#ifndef FAVE_MAC_H
#define FAVE_MAC_H

#include <cstdio>
#include <math.h>

#include <glm/glm.hpp>

#include "types.h"
#include "utils.h"
#include "buffers/vbo.h"

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
    float density;
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
    uint16_t size_x;
    uint16_t size_y;
    uint16_t total_size;

    uint16_t num_markers;

    GridCell_t *cells;
    Marker_t *markers;
    uint16_t *num_cell_markers;
    uint16_t *first_cell_marker;
    uint16_t *cell_marker_ids;

    float cell_size;
    float inv_cell_size;

    float marker_radius;
    float marker_inv_spacing;

    float density;
    float rest_density;
} MacGrid_t;

void MAC_init(MacGrid_t *grid, uint16_t size_x, uint16_t size_y, float cell_size);
void MAC_handleObstacle(MacGrid_t *grid, glm::vec3 obstaclePos, float obstacleRadius);
void MAC_update(MacGrid_t *grid, float dt);
Pair_t MAC_transformGridToVerticies(MacGrid_t *grid, Vertex_t *vertices, GLuint *indices);
Pair_t MAC_transformMarkersToVertices(MacGrid_t *grid, Vertex_t *markerVertices, GLuint *markerIndices);
void MAC_destroy(MacGrid_t *grid);

#endif // FAVE_MAC_H
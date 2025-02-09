#ifndef FAVE_MAC_H
#define FAVE_MAC_H

#include <cstdio>
#include <math.h>

#include <glm/glm.hpp>

#include "utils.h"
#include "buffers/vbo.h"

enum CellType : uint8_t
{
    FLUID = 0,
    SOLID = 1,
    AIR = 2
};

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

    uint16_t num_markers;

    GridCell **cells;
    Marker *markers;

    float cell_size;
    float inv_cell_size;

    float marker_radius;
    float marker_inv_radius;

    float density;
    float rest_density;
} MacGrid_t;

void MAC_init(MacGrid_t *grid, uint16_t size_x, uint16_t size_y, float cell_size);
void MAC_handleObstacle(MacGrid_t *grid, glm::vec3 obstaclePos, float obstacleRadius);
void MAC_update(MacGrid_t *grid, float dt);
void MAC_transformGridToVerticies(MacGrid_t *grid, Vertex *vertices, GLuint *indices);
void MAC_destroy(MacGrid_t *grid);

#endif // FAVE_MAC_H
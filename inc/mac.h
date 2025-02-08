#ifndef FAVE_MAC_H
#define FAVE_MAC_H

#include <cstdio>

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
    CellType type;
} GridCell_t;

typedef struct Marker
{
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec4 color;
    float density;
    float rest_density;
} Marker_t;

typedef struct MacGrid
{
    uint16_t size_x;
    uint16_t size_y;

    GridCell** cells;
    Marker* markers;

    float cell_size;
    float inv_cell_size;

    float marker_radius;
    float marker_inv_radius;

    float density;
} MacGrid_t;

void MAC_init(MacGrid_t *grid, uint16_t size_x, uint16_t size_y, float cell_size);
void MAC_transformGridToVerticies(MacGrid_t *grid, Vertex *vertices, GLuint *indices);

#endif // FAVE_MAC_H
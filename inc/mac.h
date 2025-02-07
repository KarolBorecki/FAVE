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

typedef struct 
{
    float p;
    float v, u, w; // v - up/down vec, u - left/right vec, w - forward/backward vec
    float dv, dy, dw;
    float prevv, prevu, prevw;
    float s;
    CellType type;
} GridCell;

typedef struct 
{
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float density;
    float rest_density;
} Marker;

typedef  struct 
{
    uint16_t size_x;
    uint16_t size_y;

    GridCell** cells;
    Marker* markers;

    float cell_size;
    float inv_cell_size;

    float marker_radius = 0.1f;
    float marker_inv_radius = 1.0f / (2.2f * marker_radius);

    float density;
} MacGrid;

void MAC_init(MacGrid *grid, uint16_t size_x, uint16_t size_y, float cell_size);
void MAC_transformGridToVerticies(MacGrid *grid, Vertex *vertices, GLuint *indices);

#endif // FAVE_MAC_H
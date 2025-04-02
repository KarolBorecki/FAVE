#ifndef FAVE_MARCHING_CUBES_H
#define FAVE_MARCHING_CUBES_H

#include <cglm/struct.h>

#ifdef __cplusplus
extern "C"
{
#endif

    extern const int edgeTable[256];
    extern int triTable[256][16];

    extern const vec3s cornerOffsets[8];
    extern const int cornerIndexAFromEdge[12];
    extern const int cornerIndexBFromEdge[12];

    extern const int edgeTable2D[16];
    extern const int triTable2D[16][4];
    extern const vec2s cornerOffsets2D[4];
    extern const int cornerIndexAFromEdge2D[4];
    extern const int cornerIndexBFromEdge2D[4];
#ifdef __cplusplus
}
#endif

#endif // FAVE_MARCHING_CUBES_H

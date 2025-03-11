#ifndef FAVE_MARCHING_CUBES_H
#define FAVE_MARCHING_CUBES_H

#include <glm/glm.hpp>

#ifdef __cplusplus
extern "C"
{
#endif

    extern const int edgeTable[256];
    extern int triTable[256][16];

    extern const glm::vec3 cornerOffsets[8];
    extern const glm::vec3 edgeVertexOffsets[12][2];
    extern const int cornerIndexAFromEdge[12];
    extern const int cornerIndexBFromEdge[12];

#ifdef __cplusplus
}
#endif

#endif // FAVE_MARCHING_CUBES_H

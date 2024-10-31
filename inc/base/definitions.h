#ifndef FAVE_DEFINITIONS_H
#define FAVE_DEFINITIONS_H

#include <glm/glm.hpp>

namespace FAVE 
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color;
        glm::vec2 texUV;
    };

    struct Color
    {
        float r;
        float g;
        float b;
        float a;
    };
    
}

#endif // FAVE_DEFINITIONS_H
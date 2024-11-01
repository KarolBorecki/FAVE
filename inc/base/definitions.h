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

    enum class LightType // NOT IMPLEMENTED
    {
        DIRECTIONAL,
        POINT,
        SPOT
    };

    enum class TextureType : uint8_t
    {
        DIFFUSE,
        SPECULAR,
        AMBIENT, // NOT IMPLEMENTED
        EMISSIVE // NOT IMPLEMENTED
    };
}

#endif // FAVE_DEFINITIONS_H
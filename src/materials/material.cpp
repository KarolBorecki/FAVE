#include "materials/material.h"

namespace FAVE 
{
    Material::Material(Shader &shader, std::vector<Texture> &textures) : m_shader(shader), m_textures(textures) {
    }
    
}
#include "materials/material.h"

namespace FAVE
{
    Material::Material(Shader &shader, Texture *p_diffuse_texture, Texture *p_specular_texture) : m_shader(shader), m_diffuse_texture(p_diffuse_texture), m_specular_texture(p_specular_texture)
    {
    }

    void Material::destroy()
    {
        m_diffuse_texture->destroy();
        m_specular_texture->destroy();
        m_shader.destroy();
    }
}
#ifndef FAVE_MATERIAL_H
#define FAVE_MATERIAL_H

#include <vector>

#include "logging/logger.h"
#include "materials/texture.h"
#include "materials/shader.h"

namespace FAVE
{
    class Material
    {
    public:
        Material(Shader &p_shader, Texture &p_diffuse_texture, Texture &p_specular_texture);
        virtual ~Material() = default;

        void destroy();

        inline Shader &shader() { return m_shader; }
        inline Texture &diffuseTexture() { return m_diffuse_texture; }
        inline Texture &specularTexture() { return m_specular_texture; }

    private:
        Shader &m_shader;
        Texture &m_diffuse_texture;
        Texture &m_specular_texture;
    };

}

#endif // FAVE_MATERIAL_H
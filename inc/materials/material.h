#ifndef FAVE_MATERIAL_H
#define FAVE_MATERIAL_H

#include <vector>

#include "materials/texture.h"
#include "materials/shader.h"

namespace FAVE
{
    class Material
    {
    public:
        Material(Shader &shader, std::vector<Texture> &textures);
        virtual ~Material() = default;

        inline Shader &shader() { return m_shader; }
        inline std::vector<Texture> &textures() { return m_textures; }

    private:
        Shader m_shader;
        std::vector<Texture> m_textures;
    };

}

#endif // FAVE_MATERIAL_H
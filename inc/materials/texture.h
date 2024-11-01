#ifndef FAVE_TEXTURE_H
#define FAVE_TEXTURE_H

#include <glad/glad.h>
#include <stb/stb_image.h>

#include "logging/exceptions.h"
#include "base/indexable.h"
#include "base/definitions.h"
#include "materials/shader.h"

namespace FAVE
{
    class Texture : Indexable
    {
    public:
        Texture(const char *p_image, TextureType p_texType, GLuint p_slot, GLenum p_format, GLenum p_pixelType);
        virtual ~Texture() = default;

        void bind();
        void unbind();
        void destroy();

        void texUnit(Shader &p_shader, const char *p_uniform, GLuint p_unit);

        TextureType type() const { return m_type; }

    protected:
        GLuint m_unit;
        TextureType m_type;
    };
}

#endif // FAVE_TEXTURE_H
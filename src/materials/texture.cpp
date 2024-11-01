#include "materials/texture.h"

namespace FAVE
{
    Texture::Texture(const char *p_image, TextureType p_texType, GLuint p_slot, GLenum p_format, GLenum p_pixelType) : m_unit(p_slot), m_type(p_texType)
    {
        int width_img, height_img, num_col_ch;
        stbi_set_flip_vertically_on_load(true);
        unsigned char *bytes = stbi_load(p_image, &width_img, &height_img, &num_col_ch, 0);

        if (bytes)
        {

            glGenTextures(1, &m_ID);
            glActiveTexture(GL_TEXTURE0 + p_slot);
            glBindTexture(GL_TEXTURE_2D, m_ID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_img, height_img, 0, p_format, p_pixelType, bytes);
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else
        {
            throwError("Failed to load texture %s", p_image);
        }

        stbi_image_free(bytes);
    }

    void Texture::texUnit(Shader &p_shader, const char *p_uniform, GLuint p_unit)
    {
        GLuint tex_uni = glGetUniformLocation(p_shader.id(), p_uniform);
        p_shader.use();
        glUniform1i(tex_uni, p_unit);
    }

    void Texture::bind()
    {
        glActiveTexture(GL_TEXTURE0 + m_unit);
        glBindTexture(GL_TEXTURE_2D, m_ID);
    }

    void Texture::unbind()
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Texture::destroy()
    {
        if (glIsTexture(m_ID))
        {
            glDeleteTextures(1, &m_ID);
            m_ID = 0;
        }
    }
}
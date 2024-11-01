#include "buffers/ebo.h"

namespace FAVE
{
    EBO::EBO(std::vector<GLuint> &p_indices)
    {
        glGenBuffers(1, &m_ID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, p_indices.size() * sizeof(GLuint), p_indices.data(), GL_STATIC_DRAW);
    }

    void EBO::bind()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
    }

    void EBO::unbind()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void EBO::destroy()
    {
        if (glIsBuffer(m_ID))
        {
            glDeleteBuffers(1, &m_ID);
            m_ID = 0;
        }
    }
}
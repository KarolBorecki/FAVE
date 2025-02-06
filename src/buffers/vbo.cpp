#include "buffers/vbo.h"

VBO::VBO(std::vector<Vertex> &p_vertices)
{
    glGenBuffers(1, &m_ID);
    glBindBuffer(GL_ARRAY_BUFFER, m_ID);
    glBufferData(GL_ARRAY_BUFFER, p_vertices.size() * sizeof(Vertex), p_vertices.data(), GL_STATIC_DRAW);
}

void VBO::bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_ID);
}

void VBO::unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::destroy()
{
    if (glIsBuffer(m_ID))
    {
        glDeleteBuffers(1, &m_ID);
        m_ID = 0;
    }
}

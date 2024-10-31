#include "buffers/vao.h"

namespace FAVE {
    VAO::VAO()
    {
        glGenVertexArrays(1, &m_ID);
    }

    void VAO::bind()
    {
        glBindVertexArray(m_ID);
    }

    void VAO::unbind()
    {
        glBindVertexArray(0);
    }

    void VAO::destroy()
    {
        glDeleteVertexArrays(1, &m_ID);
    }

    void VAO::linkAttrib(VBO &p_vbo, GLuint p_layout, GLuint p_num_components, GLenum p_type, GLsizeiptr p_stride, void *p_offset)
    {
        p_vbo.bind();
        glVertexAttribPointer(p_layout, p_num_components, p_type, GL_FALSE, p_stride, p_offset);
        glEnableVertexAttribArray(p_layout);
        p_vbo.unbind();
    }
}
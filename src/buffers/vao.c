#include "buffers/vao.h"

void VAO_init(VAO *vao)
{
    glGenVertexArrays(1, &vao->ID);
}

void VAO_bind(VAO *vao)
{
    glBindVertexArray(vao->ID);
}

void VAO_linkAttrib(GLuint layout, GLuint num_components, GLenum type, GLsizeiptr stride, void *offset)
{
    glVertexAttribPointer(layout, num_components, type, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
}

void VAO_unbind()
{
    glBindVertexArray(0);
}

void VAO_destroy(VAO *vao)
{
    if (glIsVertexArray(vao->ID))
    {
        glDeleteVertexArrays(1, &vao->ID);
        vao->ID = 0;
    }
}
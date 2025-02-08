#include "buffers/vao.h"

void VAO_init(VAO_t *vao)
{
    glGenVertexArrays(1, &vao->ID);
}

void VAO_bind(VAO_t *vao)
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

void VAO_destroy(VAO_t *vao)
{
    glDeleteVertexArrays(1, &vao->ID);
    vao->ID = 0;
}
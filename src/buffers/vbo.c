#include "buffers/vbo.h"

void VBO_init(VBO_t *vbo, size_t size)
{
    glGenBuffers(1, &vbo->ID);
    glBindBuffer(GL_ARRAY_BUFFER, vbo->ID);
    glBufferData(GL_ARRAY_BUFFER, size * sizeof(Vertex), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO_bind(VBO_t *vbo)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo->ID);
}

void VBO_update(VBO_t *vbo, Vertex_t *vertices, size_t size)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo->ID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(Vertex), vertices);
}

void VBO_unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO_destroy(VBO_t *vbo)
{
    glDeleteBuffers(1, &vbo->ID);
    vbo->ID = 0;
}
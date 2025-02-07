#include "buffers/vbo.h"

void VBO_init(VBO *vbo, size_t size)
{
    glGenBuffers(1, &vbo->ID);
    glBindBuffer(GL_ARRAY_BUFFER, vbo->ID);
    glBufferData(GL_ARRAY_BUFFER, size * sizeof(Vertex), {}, GL_STATIC_DRAW);
}

void VBO_bind(VBO *vbo)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo->ID);
}

void VBO_update(VBO *vbo, Vertex *vertices, size_t size)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo->ID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size * sizeof(Vertex), vertices);
}

void VBO_unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO_destroy(VBO *vbo)
{
    if (glIsBuffer(vbo->ID))
    {
        glDeleteBuffers(1, &vbo->ID);
        vbo->ID = 0;
    }
}
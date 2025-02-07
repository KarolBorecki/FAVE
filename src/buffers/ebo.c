#include "buffers/ebo.h"

void EBO_init(EBO *ebo, size_t size)
{
    glGenBuffers(1, &ebo->ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo->ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(GLuint), {}, GL_STATIC_DRAW);
}

void EBO_bind(EBO *ebo)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo->ID);
}

void EBO_update(EBO *ebo, GLuint *indices, size_t size)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo->ID);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size * sizeof(GLuint), indices);
}

void EBO_unbind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void EBO_destroy(EBO *ebo)
{
    if (glIsBuffer(ebo->ID))
    {
        glDeleteBuffers(1, &ebo->ID);
        ebo->ID = 0;
    }
}

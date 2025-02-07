#ifndef FAVE_VAO_H
#define FAVE_VAO_H

#include <glad/glad.h>
#include <stdlib.h>

#include "vbo.h"

typedef struct
{
	GLuint ID;
} VAO;

void VAO_init(VAO *vao);
void VAO_linkAttrib(GLuint layout, GLuint num_components, GLenum type, GLsizeiptr stride, void *offset);
void VAO_bind(VAO *vao);
void VAO_unbind();
void VAO_destroy(VAO *vao);

#endif // FAVE_VAO_H
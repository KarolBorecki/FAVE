#ifndef FAVE_VAO_H
#define FAVE_VAO_H

#include <glad/glad.h>
#include <stdlib.h>

#include "vbo.h"

typedef struct VAO
{
	GLuint ID;
} VAO_t;

void VAO_init(VAO_t *vao);
void VAO_linkAttrib(GLuint layout, GLuint num_components, GLenum type, GLsizeiptr stride, void *offset);
void VAO_bind(VAO_t *vao);
void VAO_unbind();
void VAO_destroy(VAO_t *vao);

#endif // FAVE_VAO_H
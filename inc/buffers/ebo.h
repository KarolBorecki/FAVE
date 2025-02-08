#ifndef FAVE_EBO_H
#define FAVE_EBO_H

#include <glad/glad.h>
#include <stdlib.h>

typedef struct EBO
{
	GLuint ID;
} EBO_t;

void EBO_init(EBO_t *ebo, size_t size);
void EBO_bind(EBO_t *ebo);
void EBO_update(EBO_t *ebo, GLuint *indices, size_t size);
void EBO_unbind();
void EBO_destroy(EBO_t *ebo);

#endif // FAVE_EBO_H
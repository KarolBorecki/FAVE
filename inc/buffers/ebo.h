#ifndef FAVE_EBO_H
#define FAVE_EBO_H

#include <glad/glad.h>
#include <stdlib.h>

typedef struct
{
	GLuint ID;
} EBO;

void EBO_init(EBO *ebo, size_t size);
void EBO_bind(EBO *ebo);
void EBO_update(EBO *ebo, GLuint *indices, size_t size);
void EBO_unbind();
void EBO_destroy(EBO *ebo);

#endif // FAVE_EBO_H
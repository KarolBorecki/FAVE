#ifndef FAVE_SHADER_H
#define FAVE_SHADER_H

#include <stdlib.h>
#include <stdio.h>

#include <glad/glad.h>

#include "utils.h"
typedef struct Shader
{
    GLuint ID;
} Shader_t;

void Shader_init(Shader_t *shader, const char *vertexPath, const char *fragmentPath);
void Shader_use(Shader_t *shader);
void Shader_setVector3f(Shader_t *shader, const char *name, float x, float y, float z);
void Shader_setVector4f(Shader_t *shader, const char *name, float x, float y, float z, float w);
void Shader_setMatrix4f(Shader_t *shader, const char *name, const float *matrix);
void Shader_destroy(Shader_t *shader);

#endif // FAVE_SHADER_H

#ifndef FAVE_SHADER_H
#define FAVE_SHADER_H

#include <stdlib.h>
#include <stdio.h>

#include <glad/glad.h>

typedef struct
{
    GLuint ID;
} Shader;

void Shader_init(Shader *shader, const char *vertexPath, const char *fragmentPath);
void Shader_use(Shader *shader);
void Shader_setVector3f(Shader *shader, const char *name, float x, float y, float z);
void Shader_setVector4f(Shader *shader, const char *name, float x, float y, float z, float w);
void Shader_setMatrix4f(Shader *shader, const char *name, const float *matrix);
void Shader_destroy(Shader *shader);

#endif // FAVE_SHADER_H

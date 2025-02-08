#include "buffers/shader.h"

void Shader_init(Shader_t *shader, const char *vertexPath, const char *fragmentPath)
{
    char *vertexShaderSource = loadResourceFileContent(vertexPath);
    char *fragmentShaderSource = loadResourceFileContent(fragmentPath);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const char **)&vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, (const char **)&fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    shader->ID = glCreateProgram();
    glAttachShader(shader->ID, vertexShader);
    glAttachShader(shader->ID, fragmentShader);
    glLinkProgram(shader->ID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    free(vertexShaderSource);
    free(fragmentShaderSource);
}

void Shader_use(Shader_t *shader)
{
    glUseProgram(shader->ID);
}

void Shader_setVector3f(Shader_t *shader, const char *name, float x, float y, float z)
{
    glUniform3f(glGetUniformLocation(shader->ID, name), x, y, z);
}

void Shader_setVector4f(Shader_t *shader, const char *name, float x, float y, float z, float w)
{
    glUniform4f(glGetUniformLocation(shader->ID, name), x, y, z, w);
}

void Shader_setMatrix4f(Shader_t *shader, const char *name, const float *matrix)
{
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, name), 1, GL_FALSE, matrix);
}

void Shader_destroy(Shader_t *shader)
{
    glDeleteProgram(shader->ID);
}
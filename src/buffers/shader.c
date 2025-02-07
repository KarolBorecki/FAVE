#include "buffers/shader.h"

static char *loadShaderFile(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Error loading shader: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(fileSize + 1);
    if (!buffer)
    {
        fprintf(stderr, "Memory allocation failed for shader file\n");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, fileSize, file);
    buffer[fileSize] = '\0';

    fclose(file);
    return buffer;
}

void Shader_init(Shader *shader, const char *vertexPath, const char *fragmentPath)
{
    char *vertexShaderSource = loadShaderFile(vertexPath);
    char *fragmentShaderSource = loadShaderFile(fragmentPath);

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

void Shader_use(Shader *shader)
{
    glUseProgram(shader->ID);
}

void Shader_setVector3f(Shader *shader, const char *name, float x, float y, float z)
{
    glUniform3f(glGetUniformLocation(shader->ID, name), x, y, z);
}

void Shader_setVector4f(Shader *shader, const char *name, float x, float y, float z, float w)
{
    glUniform4f(glGetUniformLocation(shader->ID, name), x, y, z, w);
}

void Shader_setMatrix4f(Shader *shader, const char *name, const float *matrix)
{
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, name), 1, GL_FALSE, matrix);
}

void Shader_destroy(Shader *shader)
{
    glDeleteProgram(shader->ID);
}
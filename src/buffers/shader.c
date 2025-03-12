#include "buffers/shader.h"

void Shader_init(Shader_t *shader, const char *vertexPath, const char *fragmentPath)
{
    GLint success;

    char *vertexShaderSource = loadResourceFileContent(vertexPath);
    char *fragmentShaderSource = loadResourceFileContent(fragmentPath);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const char **)&vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[512];
        glGetShaderInfoLog(vertexShader, sizeof(log), NULL, log);
        fprintf(stderr, "error compiling vertex shader:\n%s\n", log);
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, (const char **)&fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[512];
        glGetShaderInfoLog(fragmentShader, sizeof(log), NULL, log);
        fprintf(stderr, "error compiling fragment shader:\n%s\n", log);
    }

    shader->ID = glCreateProgram();
    glAttachShader(shader->ID, vertexShader);
    glAttachShader(shader->ID, fragmentShader);
    glLinkProgram(shader->ID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    free(vertexShaderSource);
    free(fragmentShaderSource);

    Shader_use(shader);
    Shader_setVector3f(shader, "scale", 1.0f, 1.0f, 1.0f);
    Shader_setVector3f(shader, "rotation", 0.0f, 0.0f, 0.0f);
    Shader_setVector3f(shader, "position", 0.0f, 0.0f, 0.0f);

    Shader_setVector4f(shader, "lightColor", 1.0f, 1.0f, 1.0f, 1.0f);
    Shader_setVector3f(shader, "lightPos", 0.0f, 10.0f, 0.0f);
}

void Shader_use(Shader_t *shader)
{
    if (!shader || shader->ID == 0)
    {
        fprintf(stderr, "Błąd: Shader nie został poprawnie zainicjalizowany\n");
        return;
    }

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
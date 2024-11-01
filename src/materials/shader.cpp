#include "materials/shader.h"

namespace FAVE
{
    Shader::Shader(const char *p_vertexPath, const char *p_fragmentPath)
    {
        std::string vertexCode = get_file_contents(p_vertexPath);
        std::string fragmentCode = get_file_contents(p_fragmentPath);

        const char *vertexSource = vertexCode.c_str();
        const char *fragmentSource = fragmentCode.c_str();

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        glCompileShader(vertexShader);
        compileErrors(vertexShader, "VERTEX");

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);
        compileErrors(fragmentShader, "FRAGMENT");

        m_ID = glCreateProgram();
        glAttachShader(m_ID, vertexShader);
        glAttachShader(m_ID, fragmentShader);
        glLinkProgram(m_ID);
        compileErrors(m_ID, "PROGRAM");

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void Shader::use()
    {
        glUseProgram(m_ID);
    }

    void Shader::unuse()
    {
        glUseProgram(0);
    }

    void Shader::destroy()
    {
        if (glIsProgram(m_ID))
        {
            glDeleteProgram(m_ID);
            m_ID = 0;
        }
    }

    void Shader::setBool(const char *p_name, bool p_value) const
    {
        glUniform1i(glGetUniformLocation(m_ID, p_name), (int)p_value);
    }

    void Shader::setInt(const char *p_name, int p_value) const
    {
        glUniform1i(glGetUniformLocation(m_ID, p_name), p_value);
    }

    void Shader::setFloat(const char *p_name, float p_value) const
    {
        glUniform1f(glGetUniformLocation(m_ID, p_name), p_value);
    }

    void Shader::compileErrors(unsigned int p_shader, const char *p_type)
    {
        int success;
        char infoLog[1024];
        if (strcmp(p_type, "PROGRAM") != 0)
        {
            glGetShaderiv(p_shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(p_shader, 1024, NULL, infoLog);
                throwError("ERROR::SHADER_COMPILATION_ERROR of type: %s\n%s\n -- --------------------------------------------------- --\n", p_type, infoLog);
            }
        }
        else
        {
            glGetProgramiv(p_shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(p_shader, 1024, NULL, infoLog);
                throwError("ERROR::PROGRAM_LINKING_ERROR of type: %s\n%s\n -- --------------------------------------------------- --\n", p_type, infoLog);
            }
        }
    }
}
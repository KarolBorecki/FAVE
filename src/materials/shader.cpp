#include "materials/shader.h"

namespace FAVE
{
    Shader::Shader(const char *p_vertexPath, const char *p_fragmentPath)
    {
        // std::string vertexCode = get_file_contents(p_vertexPath);
        // std::string fragmentCode = get_file_contents(p_fragmentPath);

        // const char *vShaderCode = vertexCode.c_str();
        // const char *fShaderCode = fragmentCode.c_str();

        // unsigned int vertex, fragment;

        // vertex = glCreateShader(GL_VERTEX_SHADER);
        // glShaderSource(vertex, 1, &vShaderCode, NULL);
        // glCompileShader(vertex);
        // compileErrors(vertex, "VERTEX");

        // fragment = glCreateShader(GL_FRAGMENT_SHADER);
        // glShaderSource(fragment, 1, &fShaderCode, NULL);
        // glCompileShader(fragment);
        // compileErrors(fragment, "FRAGMENT");

        // m_ID = glCreateProgram();
        // glAttachShader(m_ID, vertex);
        // glAttachShader(m_ID, fragment);
        // glLinkProgram(m_ID);
        // compileErrors(m_ID, "PROGRAM");

        // glDeleteShader(vertex);
        // glDeleteShader(fragment);

        // Read vertexFile and fragmentFile and store the strings
        std::string vertexCode = get_file_contents(p_vertexPath);
        std::string fragmentCode = get_file_contents(p_fragmentPath);

        // Convert the shader source strings into character arrays
        const char *vertexSource = vertexCode.c_str();
        const char *fragmentSource = fragmentCode.c_str();

        // Create Vertex Shader Object and get its reference
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        // Attach Vertex Shader source to the Vertex Shader Object
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        // Compile the Vertex Shader into machine code
        glCompileShader(vertexShader);
        // Checks if Shader compiled succesfully
        compileErrors(vertexShader, "VERTEX");

        // Create Fragment Shader Object and get its reference
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        // Attach Fragment Shader source to the Fragment Shader Object
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        // Compile the Vertex Shader into machine code
        glCompileShader(fragmentShader);
        // Checks if Shader compiled succesfully
        compileErrors(fragmentShader, "FRAGMENT");

        // Create Shader Program Object and get its reference
        m_ID = glCreateProgram();
        // Attach the Vertex and Fragment Shaders to the Shader Program
        glAttachShader(m_ID, vertexShader);
        glAttachShader(m_ID, fragmentShader);
        // Wrap-up/Link all the shaders together into the Shader Program
        glLinkProgram(m_ID);
        // Checks if Shaders linked succesfully
        compileErrors(m_ID, "PROGRAM");

        // Delete the now useless Vertex and Fragment Shader objects
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    Shader::~Shader()
    {
        glDeleteProgram(m_ID);
    }

    void Shader::use()
    {
        glUseProgram(m_ID);
    }

    void Shader::unuse()
    {
        glUseProgram(0);
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
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << p_type << "\n"
                          << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(p_shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(p_shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << p_type << "\n"
                          << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
}
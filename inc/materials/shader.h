#ifndef FAVE_SHADER_H
#define FAVE_SHADER_H

#include <cstring>
#include <glad/glad.h>

#include "base/indexable.h"
#include "util/file_utils.h"

namespace FAVE
{
    class Shader : public Indexable
    {
    public:
        Shader(const char *p_vertexPath, const char *p_fragmentPath);
        ~Shader();

        void use();
        void unuse();

        void setBool(const char *p_name, bool p_value) const;
        void setInt(const char *p_name, int p_value) const;
        void setFloat(const char *p_name, float p_value) const;

    private:
        void compileErrors(unsigned int p_shader, const char *p_type);
    };
}

#endif // FAVE_SHADER_H
#ifndef FAVE_VAO_H
#define FAVE_VAO_H

#include <glad/glad.h>

#include "buffers/buffer.h"
#include "buffers/vbo.h"

namespace FAVE
{
	class VAO : public Buffer
	{
	public:
		VAO();

		void bind() override;
		void unbind() override;
		void destroy() override;

		void linkAttrib(VBO &p_VBO, GLuint p_layout, GLuint p_num_components, GLenum p_type, GLsizeiptr p_stride, void *p_offset);
	};
}

#endif // FAVE_VAO_H
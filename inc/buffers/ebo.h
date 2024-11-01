#ifndef FAVE_EBO_H
#define FAVE_EBO_H

#include <vector>

#include <glad/glad.h>

#include "logging/logger.h"
#include "buffers/buffer.h"

namespace FAVE
{
	class EBO : public Buffer
	{
	public:
		EBO(std::vector<GLuint> &indices);
		virtual ~EBO() = default;

		void bind() override;
		void unbind() override;
		void destroy() override;
	};
}

#endif // FAVE_EBO_H
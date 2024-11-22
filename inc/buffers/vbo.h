#ifndef FAVE_VBO_H
#define FAVE_VBO_H

#include <vector>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "logging/logger.h"
#include "base/definitions.h"
#include "buffers/buffer.h"

namespace FAVE
{
	class VBO : public Buffer
	{
	public:
		VBO(std::vector<Vertex> &p_vertices);

		void bind() override;
		void unbind() override;
		void destroy() override;
	};
}

#endif // FAVE_VBO_H
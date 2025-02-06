#ifndef FAVE_VBO_H
#define FAVE_VBO_H

#include <vector>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "buffers/buffer.h"

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 texUV;
};

class VBO : public Buffer
{
public:
	VBO(std::vector<Vertex> &p_vertices);

	void bind() override;
	void unbind() override;
	void destroy() override;
};

#endif // FAVE_VBO_H
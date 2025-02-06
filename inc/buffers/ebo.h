#ifndef FAVE_EBO_H
#define FAVE_EBO_H

#include <vector>

#include <glad/glad.h>

#include "buffers/buffer.h"

class EBO : public Buffer
{
public:
	EBO(std::vector<GLuint> &indices);

	void bind() override;
	void unbind() override;
	void destroy() override;
};

#endif // FAVE_EBO_H
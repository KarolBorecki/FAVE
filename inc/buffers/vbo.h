#ifndef FAVE_VBO_H
#define FAVE_VBO_H

#include <glad/glad.h>
#include <glm/glm.hpp>

typedef struct
{
	GLuint ID;
} VBO;

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 texUV;
};

void VBO_init(VBO *vbo, size_t size);
void VBO_bind(VBO *vbo);
void VBO_update(VBO *vbo, Vertex *vertices, size_t size);
void VBO_unbind();
void VBO_destroy(VBO *vbo);

#endif // FAVE_VBO_H
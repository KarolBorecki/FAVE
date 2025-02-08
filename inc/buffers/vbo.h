#ifndef FAVE_VBO_H
#define FAVE_VBO_H

#include <glad/glad.h>
#include <glm/glm.hpp>

typedef struct VBO
{
	GLuint ID;
} VBO_t;

typedef struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 texUV;
} Vertex_t;

void VBO_init(VBO_t *vbo, size_t size);
void VBO_bind(VBO_t *vbo);
void VBO_update(VBO_t *vbo, Vertex_t *vertices, size_t size);
void VBO_unbind();
void VBO_destroy(VBO_t *vbo);

#endif // FAVE_VBO_H
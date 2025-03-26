#ifndef FAVE_VBO_H
#define FAVE_VBO_H

#include <glad/glad.h>
#include <cglm/struct.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct VBO
	{
		GLuint ID;
	} VBO_t;

	typedef struct Vertex
	{
		vec3s position;
		vec3s normal;
		vec3s color;
		vec2s texUV;
	} Vertex_t;

	void VBO_init(VBO_t *vbo, size_t size);
	void VBO_bind(VBO_t *vbo);
	void VBO_update(VBO_t *vbo, Vertex_t *vertices, size_t size);
	void VBO_unbind();
	void VBO_destroy(VBO_t *vbo);

#ifdef __cplusplus
}
#endif

#endif // FAVE_VBO_H

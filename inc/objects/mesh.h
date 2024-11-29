#ifndef FAVE_MESH_H
#define FAVE_MESH_H

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "logging/logger.h"
#include "base/definitions.h"

#include "buffers/vao.h"
#include "buffers/vbo.h"
#include "buffers/ebo.h"

#include "objects/camera.h"
#include "objects/render_object.h"

#include "materials/material.h"

namespace FAVE
{
    class Mesh : public RenderObject
    {
    public:
        Mesh(std::vector<Vertex> &p_vertices, std::vector<GLuint> &p_indices, Material &p_material);

        void draw(float p_delta_time, Camera *p_camera, Light *p_light) override;
        void destroy() override;

    protected:
        VAO m_vao;

        std::vector<Vertex> m_vertices;
        std::vector<GLuint> m_indices;

        Material &m_material;
    };
}

#endif // FAVE_MESH_H
#ifndef FAVE_MESH_H
#define FAVE_MESH_H

#include "logging/logger.h"
#include "base/definitions.h"
#include "materials/material.h"
#include "buffers/vao.h"
#include "buffers/ebo.h"
#include "objects/camera.h"
#include "objects/render_object.h"

namespace FAVE
{
    class Mesh : public RenderObject
    {
    public:
        Mesh(std::vector<Vertex> &p_vertices, std::vector<GLuint> &p_indices, Material &p_material);
        virtual ~Mesh() = default;

        void draw(Camera *p_camera, Light *p_light) override;
        void destroy() override;

    protected:
        VAO m_vao;

        std::vector<Vertex> m_vertices;
        std::vector<GLuint> m_indices;

        Material &m_material;
    };
}

#endif // FAVE_MESH_H
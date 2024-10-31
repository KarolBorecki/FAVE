#ifndef FAVE_MESH_H
#define FAVE_MESH_H

#include "buffers/vao.h"
#include "buffers/ebo.h"
#include "materials/material.h"
#include "objects/camera.h"
#include "objects/render_object.h"

namespace FAVE
{
    class Mesh : public RenderObject
    {
    public:
        Mesh(std::vector<Vertex> &p_vertices, std::vector<GLuint> &p_indices, Material *p_material);
        virtual ~Mesh() = default;

        void draw(Camera *p_camera, Light *p_light) override;

    protected:
        VAO m_vao;

        std::vector<Vertex> m_vertices;
        std::vector<GLuint> m_indices;

        Material *m_material {nullptr};
    };
}

#endif // FAVE_MESH_H
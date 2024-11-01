#include "objects/mesh.h"

namespace FAVE
{
    Mesh::Mesh(std::vector<Vertex> &p_vertices, std::vector<GLuint> &p_indices, Material &p_material) : m_vertices(p_vertices), m_indices(p_indices), m_material(p_material)
    {
        m_vao.bind();
        VBO vbo(m_vertices);
        EBO ebo(m_indices);
        m_vao.linkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void *)0);
        m_vao.linkAttrib(vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void *)(3 * sizeof(float)));
        m_vao.linkAttrib(vbo, 2, 3, GL_FLOAT, sizeof(Vertex), (void *)(6 * sizeof(float)));
        m_vao.linkAttrib(vbo, 3, 2, GL_FLOAT, sizeof(Vertex), (void *)(9 * sizeof(float)));
        m_vao.unbind();
        vbo.unbind();
        ebo.unbind();

        m_material.shader().use();
    }

    void Mesh::draw(Camera *p_camera, Light *p_light)
    {
        m_material.shader().use();
        glUniformMatrix4fv(glGetUniformLocation(m_material.shader().id(), "model"), 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(1.0f), position())));
        glUniform4f(glGetUniformLocation(m_material.shader().id(), "lightColor"), p_light->color().r, p_light->color().g, p_light->color().b, p_light->color().a);
        glUniform3f(glGetUniformLocation(m_material.shader().id(), "lightPos"), p_light->position().x, p_light->position().y, p_light->position().z);
        m_vao.bind();

        m_material.diffuseTexture().texUnit(m_material.shader(), "diffuse0", 0);
        m_material.diffuseTexture().bind();
        m_material.diffuseTexture().texUnit(m_material.shader(), "specular0", 1);
        m_material.diffuseTexture().bind();

        glUniform3f(glGetUniformLocation(m_material.shader().id(), "camPos"), p_camera->position().x, p_camera->position().y, p_camera->position().z);
        p_camera->matrix(m_material.shader(), "camMatrix");

        glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
    }

    void Mesh::destroy()
    {
        m_vao.destroy();
        m_material.destroy();
    }
}
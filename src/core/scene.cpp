#include "core/scene.h"

namespace FAVE
{
    void Scene::render()
    {
        for (auto &object : m_objects)
        {
            object->draw(m_camera, m_light);
        }
    }

    void Scene::addObject(RenderObject *object)
    {
        m_objects.push_back(object);
    }

    void Scene::setCamera(Camera *camera)
    {
        m_camera = camera;
        m_camera->setSize(m_width, m_height);
    }

    void Scene::setLight(Light *light)
    {
        m_light = light;
    }

    void Scene::onWindowResize(uint16_t width, uint16_t height)
    {
        m_width = width;
        m_height = height;
        m_camera->setSize(m_width, m_height);
    }

    void Scene::finish()
    {
        for (auto &object : m_objects)
            object->destroy();
    }
}
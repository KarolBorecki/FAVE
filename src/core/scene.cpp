#include "core/scene.h"

namespace FAVE
{
    void Scene::init()
    {
        m_camera->start();
        m_light->start();
        for (auto &object : m_objects)
            object->start();
    }

    void Scene::render(float p_delta_time) // TODO deltaTime
    {
        for (auto &object : m_objects)
        {
            object->draw(p_delta_time, m_camera, m_light);
            object->update(p_delta_time); // TODO isnt it gonna slow down drawing process?

            m_camera->update(p_delta_time);
            m_light->update(p_delta_time);
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
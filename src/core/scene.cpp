#include "core/scene.h"

namespace FAVE
{
    void Scene::init()
    {
        if (m_camera == nullptr)
            throw std::runtime_error("Camera is not set");
        m_camera->start();
        m_camera->startScripts();

        if (m_light == nullptr)
            throw std::runtime_error("Light is not set");
        m_light->start();
        m_light->startScripts();

        for (auto &object : m_objects)
        {
            object->start();
            object->startScripts();
        }
    }

    void Scene::render(float p_delta_time) // TODO deltaTime
    {
        for (auto &object : m_objects)
        {
            object->update(p_delta_time); // TODO isnt it gonna slow down drawing process?
            object->fixedUpdate(m_fixed_delta_time);
            object->updateScripts(p_delta_time);
            object->fixedUpdateScripts(m_fixed_delta_time);
            object->draw(p_delta_time, m_camera, m_light);

            m_camera->update(p_delta_time);
            m_camera->fixedUpdate(m_fixed_delta_time);
            m_camera->updateScripts(p_delta_time);
            m_camera->fixedUpdateScripts(m_fixed_delta_time);

            m_light->update(p_delta_time);
            m_light->fixedUpdate(m_fixed_delta_time);
            m_light->updateScripts(p_delta_time);
            m_light->fixedUpdateScripts(m_fixed_delta_time);
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
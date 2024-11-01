#ifndef FAVE_SCENE_H
#define FAVE_SCENE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "objects/render_object.h"
#include "objects/camera.h"
#include "objects/light.h"

namespace FAVE
{
    class Scene
    {
    public:
        Scene() = default;
        virtual ~Scene() = default;

        void render(float p_delta_time);

        void addObject(RenderObject *p_obj);

        void setCamera(Camera *p_camera);
        inline Camera *camera() { return m_camera; }

        void setLight(Light *p_light);
        inline Light *light() { return m_light; }

        void onWindowResize(uint16_t p_width, uint16_t p_height);

        void finish();

    private:
        std::vector<RenderObject *> m_objects;
        Camera *m_camera;
        Light *m_light;

        uint16_t m_width{1};
        uint16_t m_height{1};
    };
}

#endif // FAVE_SCENE_H
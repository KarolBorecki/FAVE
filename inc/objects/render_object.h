#ifndef FAVE_RENDER_OBJECT_H
#define FAVE_RENDER_OBJECT_H

#include "objects/object.h"
#include "objects/camera.h"
#include "objects/light.h"
#include "materials/shader.h"

namespace FAVE 
{
    class RenderObject : public Object
    {
    public:
        RenderObject() = default;
        virtual ~RenderObject() = default;

        virtual void draw(Camera *p_camera, Light* p_light) = 0;
    };
}

#endif // FAVE_RENDER_OBJECT_H
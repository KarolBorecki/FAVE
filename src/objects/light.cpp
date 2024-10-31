#include "objects/light.h"

namespace FAVE
{
    Light::Light(const Color &p_color) : m_color(p_color) {}

    void Light::setColor(const Color &p_color)
    {
        m_color = p_color;
    }
}
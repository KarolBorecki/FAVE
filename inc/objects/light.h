#ifndef FAVE_LIGHT_H
#define FAVE_LIGHT_H

#include "base/definitions.h"
#include "objects/object.h"

namespace FAVE 
{
    class Light : public Object
    {
    public:
        Light(const Color &p_color);
        virtual ~Light() = default;

        void setColor(const Color &p_color);
        inline Color &color() { return m_color; }

    private:
        Color m_color;
    };
}

#endif // FAVE_LIGHT_H
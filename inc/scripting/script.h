#ifndef FAVE_SCRIPT_H
#define FAVE_SCRIPT_H

#include "logging/logger.h"

namespace FAVE
{
    class Object;
}

namespace FAVE
{
    class Script
    {
    public:
        Script() = default;

        virtual void start() {};

        virtual void update(float p_delta_time) {};

        void attachTo(Object *p_object) { m_object = p_object; }

    protected:
        Object *m_object{nullptr};
    };
}

#endif /* FAVE_SCRIPT_H */
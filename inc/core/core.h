#ifndef FAVE_CORE_H
#define FAVE_CORE_H

#include "core/window.h"
#include "logging/exceptions.h"

namespace FAVE
{
    class Core
    {
    public:
        Core() = default;
        virtual ~Core() = default;

        static void init();
        static void finish();
    };
}

#endif // FAVE_CORE_H
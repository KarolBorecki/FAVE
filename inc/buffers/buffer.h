#ifndef FAVE_BUFFER_H
#define FAVE_BUFFER_H

#include <glad/glad.h>

#include "base/indexable.h"

namespace FAVE
{
    class Buffer : public Indexable
    {
    public:
        Buffer() = default;
        virtual ~Buffer() = default;

        virtual void bind() = 0;
        virtual void unbind() = 0;
        virtual void destroy() = 0;
    };
}

#endif // FAVE_BUFFER_H
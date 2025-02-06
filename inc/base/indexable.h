#ifndef FAVE_INDEXABLE_H
#define FAVE_INDEXABLE_H

#include "base/indexable.h"

class Indexable
{
public:
    Indexable() = default;

    GLuint id() const { return m_ID; }

protected:
    GLuint m_ID;
};

#endif // FAVE_INDEXABLE_H
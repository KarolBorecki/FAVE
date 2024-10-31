#ifndef FAVE_EXCEPTIONS_H
#define FAVE_EXCEPTIONS_H

#include <stdexcept>

#include "logging/logger.h"

namespace FAVE
{
    /**
     * @brief Throws an error and ends the engine work immedietly. It also prints message in red color on the console.
     *
     * @param p_format - message format in c-style.
     * @param ...  - format args.
     */
    void throwError(const char *p_format, ...);

    /**
     * @brief Throws a warning. It also prints message in yellow color on the console.
     *
     * @param p_format - message format in c-style.
     * @param ...  - format args.
     */
    void throwWarning(const char *p_format, ...);
}

#endif // FAVE_EXCEPTIONS_H
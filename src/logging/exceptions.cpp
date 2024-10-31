#include "logging/exceptions.h"

namespace FAVE
{
    void throwError(const char *p_format, ...)
    {
        va_list args;
        printf("\033[%dm", FontColor::RED);
        va_start(args, p_format);
        vprintf(p_format, args);
        printf("\n");
        va_end(args);
        throw std::runtime_error("");
    }

    void throwWarning(const char *p_format, ...)
    {
        va_list args;
        printf("\033[%dm", FontColor::YELLOW);
        va_start(args, p_format);
        vprintf(p_format, args);
        printf("\n");
        va_end(args);
    }
}
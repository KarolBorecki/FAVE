#include "logging/logger.h"

namespace FAVE
{
    void error(const char *p_format, ...)
    {
        setFont(FontColor::RED);
        va_list args;
        va_start(args, p_format);
        vprintf(p_format, args);
        printf("\n");
        va_end(args);
    }

    void warning(const char *p_format, ...)
    {
        setFont(FontColor::YELLOW);
        va_list args;
        va_start(args, p_format);
        vprintf(p_format, args);
        printf("\n");
        va_end(args);
    }

    void info(const char *p_format, ...)
    {
        setFont(FontColor::BLUE);
        va_list args;
        va_start(args, p_format);
        vprintf(p_format, args);
        printf("\n");
        va_end(args);
    }

    void log()
    {
        setFont(FontColor::WHITE);
        printf("\n");
    }

    void log(const char *p_format, ...)
    {
        setFont(FontColor::WHITE);
        va_list args;
        va_start(args, p_format);
        vprintf(p_format, args);
        printf("\n");
        va_end(args);
    }

    void log(int p_color_code, const char *p_format, ...)
    {
        setFont(p_color_code);
        va_list args;
        va_start(args, p_format);
        vprintf(p_format, args);
        printf("\n");
        va_end(args);
    }

    void logInline(const char *p_format, ...)
    {
        setFont(FontColor::WHITE);
        va_list args;
        va_start(args, p_format);
        vprintf(p_format, args);
        va_end(args);
    }

    void setFont(int p_color_code, int p_style_code)
    {
        printf("\033[%dm", FontColor::WHITE);
        printf("\033[%dm", p_style_code);
        printf("\033[%dm", p_color_code);
    }

    void setFont(int colorCode)
    {
        setFont(colorCode, FontStyle::NORMAL);
    }
}
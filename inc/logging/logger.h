#ifndef FAVE_LOGGER_H
#define FAVE_LOGGER_H

#include <cstdarg>
#include <cstring>
#include <cstdio>

namespace FAVE
{
    /**
     * @brief representing console color codes.
     */
    enum FontColor
    {
        WHITE = 0,
        GREY = 37,
        LIGHT_BLUE = 36,
        PINK = 35,
        GREEN = 32,
        BLUE = 34,  /* RESERVED FOR INFOS */
        RED = 31,   /* RESERVED FOR WARNINGS */
        YELLOW = 33 /* RESERVED FOR WARNINGS */
    };

    /**
     * @brief Enumeration representing console font styles codes.
     */
    enum FontStyle
    {
        NORMAL = 0,
        BOLD = 1
    };

    /**
     * @brief Logs onto the console in red color.
     */
    void error(const char *p_format, ...);

    /**
     * @brief Logs onto the console in yellow color.
     */
    void warning(const char *p_format, ...);

    /**
     * @brief Logs onto the console in blue color.
     */
    void info(const char *p_format, ...);

    /**
     * @brief Prints empty line onto the console.
     */
    void log();

    /**
     * @brief Logs onto the console with standard printf-like interface.
     */
    void log(const char *p_format, ...);

    /**
     * @brief Logs onto the console in specified colorCode color.
     */
    void log(int p_color_code, const char *p_format, ...);

    /**
     * @brief Logs onto the console.
     */
    void logInline(const char *p_format, ...);

    /**
     * @brief Sets console color to colorCode and console font style to styleCode;
     */
    void setFont(int p_color_code, int p_style_code);

    /**
     * @brief Sets console color to colorCode.
     */
    void setFont(int p_color_code);
}

#endif // FAVE_LOGGER_H
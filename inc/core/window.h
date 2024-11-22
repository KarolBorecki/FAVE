#ifndef FAVE_WINDOW_H
#define FAVE_WINDOW_H

#include <cstdint>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "core/scene.h"
#include "logging/exceptions.h"

namespace FAVE
{
    class Window
    {
    public:
        Window() = default;

        static void open(uint16_t width, uint16_t height, const char *title);
        static void start();
        static void close();

        static void setScene(Scene *scene);
        static inline Scene *scene() { return m_scene; }

        static inline void setBackgroundColor(const Color &color) { m_background_color = color; }

        static inline GLFWwindow *getGLFWwindow() { return m_window; }

    private:
        static uint16_t m_width;
        static uint16_t m_height;
        static const char *m_title;
        static Color m_background_color;

        static Scene *m_scene;
        static GLFWwindow *m_window;

        static void resizeCallback(GLFWwindow *p_window, int p_width, int p_height);
    };
}

#endif // FAVE_WINDOW_H
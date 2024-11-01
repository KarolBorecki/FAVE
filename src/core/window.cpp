#include "core/window.h"

namespace FAVE
{
    uint16_t Window::m_width = 1;
    uint16_t Window::m_height = 1;
    const char *Window::m_title = "FAVE";
    Color Window::m_background_color = {0.1f, 0.1f, 0.1f, 1.0f};
    Scene *Window::m_scene = nullptr;
    GLFWwindow *Window::m_window = nullptr;

    void Window::open(uint16_t p_width, uint16_t p_height, const char *p_title)
    {
        m_width = p_width;
        m_height = p_height;
        m_title = p_title;
        m_window = glfwCreateWindow(m_width, m_height, m_title, nullptr, nullptr);
        if (!m_window)
        {
            glfwTerminate();
            throwError("Failed to create window");
        }

        glfwMakeContextCurrent(m_window);
        gladLoadGL();
        glViewport(0, 0, m_width, m_height);

        glfwSetFramebufferSizeCallback(m_window, resizeCallback);
    }

    void Window::start() // TODO deltaTime
    {
        if (!m_window)
        {
            throwError("Window is not created");
        }

        if (!m_scene)
        {
            throwError("Scene is not set");
        }

        if (!m_scene->camera())
        {
            throwError("Camera is not set");
        }

        glEnable(GL_DEPTH_TEST);

        while (!glfwWindowShouldClose(m_window))
        {
            glClearColor(m_background_color.r, m_background_color.g, m_background_color.b, m_background_color.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            m_scene->camera()->update_matrix();

            m_scene->render(0.001f);

            glfwSwapBuffers(m_window);
            glfwPollEvents();
        }
    }

    void Window::close()
    {
        m_scene->finish();
        glfwDestroyWindow(m_window);
    }

    void Window::setScene(Scene *scene)
    {
        m_scene = scene;
        m_scene->onWindowResize(m_width, m_height);
    }

    void Window::resizeCallback(GLFWwindow *p_window, int p_width, int p_height)
    {
        m_width = p_width;
        m_height = p_height;
        glViewport(0, 0, m_width, m_height);
        m_scene->onWindowResize(m_width, m_height);
    }
}
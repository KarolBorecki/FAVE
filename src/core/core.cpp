#include "core/core.h"

namespace FAVE {
    void Core::init() {
        if (!glfwInit()) {
            glfwTerminate();
            throwError("Failed to initialize GLFW");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    void Core::finish()
    {
        glfwTerminate();
    }
}
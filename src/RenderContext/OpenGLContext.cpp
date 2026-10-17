#include "OpenGLContext.h"
#include "gpu/DebugOutput.h"

#include <cstdlib>
#include <iostream>

namespace OpenGlContext
{
    void processInput(GLFWwindow *window)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }

    void preRender(GLFWwindow *window)
    {
        glfwPollEvents();
        processInput(window);
    }

    void postRender(GLFWwindow *window)
    {
        glfwSwapBuffers(window);
    }

    // Initialize GLFW, the GL 4.5 context and glad
    // Must be the first thing to launch to allow function to be found
    bool initGlContext(GLFWwindow *&window, int width, int height)
    {
        glfwSetErrorCallback([](int error, const char *description)
                             { std::cerr << "GLFW Error (" << error << "): " << description << std::endl; });

        // Multi-viewports need window positions, which Wayland forbids: prefer XWayland
        if (glfwPlatformSupported(GLFW_PLATFORM_X11) && std::getenv("DISPLAY"))
            glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);

        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef NDEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

        window = glfwCreateWindow(width, height, "ImgEditor", NULL, NULL);
        if (window == NULL)
        {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(window);

        if (!gladLoadGL(glfwGetProcAddress))
        {
            std::cerr << "Failed to load the OpenGL 4.5 functions" << std::endl;
            glfwTerminate();
            return false;
        }
#ifndef NDEBUG
        if (!gpu::enableDebugOutput())
            std::cerr << "No GL debug context: GL errors will go unreported" << std::endl;
#endif

        //(VSync)
        glfwSwapInterval(1);
        return true;
    }

    void quit(GLFWwindow *window)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

}

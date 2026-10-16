
#include "OpenGLContext.h"
#include <cstdlib>

namespace OpenGlContext
{
    void processInput(GLFWwindow *window)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }

    void preRender()
    {
        glfwPollEvents();
    }

    void postRender(GLFWwindow *window)
    {

        glfwSwapBuffers(window);

        processInput(window);
        glClearColor(0.2f, 0.2f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT*/);
    }

    // Initialize GLFW, the GL 4.5 context and glad
    // Must be the first thing to launch to allow function to be found
    bool initGlContext(GLFWwindow *&window, int width, int height)
    {
        // Context initialization
        // GLFW Init

        glfwSetErrorCallback([](int error, const char *description)
                             { std::cerr << "GLFW Error (" << error << "): " << description << std::endl; });

        // Multi-viewports need window positions, which Wayland forbids: prefer XWayland
        if (glfwPlatformSupported(GLFW_PLATFORM_X11) && std::getenv("DISPLAY"))
            glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);

        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return 0;
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(width, height, "ImgEditor", NULL, NULL);
        if (window == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return 0;
        }
        glfwMakeContextCurrent(window);

        if (!gladLoadGL(glfwGetProcAddress))
        {
            std::cerr << "Failed to load the OpenGL 4.5 functions" << std::endl;
            glfwTerminate();
            return 0;
        }
        // Set the callback function for window resize

        glfwSetFramebufferSizeCallback(window, [](GLFWwindow *, int width, int height)
                                       { glViewport(0, 0, width, height); });

        //(VSync)
        glfwSwapInterval(1);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearDepth(1);
        GLenum error = glGetError();

        if (error != GL_NO_ERROR)
        {
            std::cerr << "Error after GLcontext initialization: " << error << std::endl;
            return 0;
        }

        return 1;
    }

    // TODO
    // TODO
    // TODO
    // TODO Clean everyhting

    void quit(GLFWwindow *window)
    { // List of each
      // Loop through this
        /*freeBuffers(model1);
        freeTexture(imageress);*/
        glfwDestroyWindow(window);
        glfwTerminate();
    }

}

#include "OpenGLContext.h"

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

    // Initialize GLEW, GL and GLFW
    // Must be the first thing to launch to allow function to be found
    bool initGlContext(GLFWwindow *&window, int width, int height)
    {
        // Context initialization
        // GLFW Init

        glfwSetErrorCallback([](int error, const char *description)
                             { std::cerr << "GLFW Error (" << error << "): " << description << std::endl; });

        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return 0;
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(width, height, "ImgEditor", NULL, NULL);
        if (window == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return 0;
        }
        glfwMakeContextCurrent(window);

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
        {
            std::cerr << "Failed to initialize GLEW" << std::endl;
            glfwTerminate();
            return 0;
        }
        // Set the callback function for window resize

        glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height)
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
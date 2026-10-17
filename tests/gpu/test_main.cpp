#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include "gpu/DebugOutput.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cstdio>

// A GL 4.5 debug context with no display: GLFW's null platform over EGL
int main(int argc, char **argv)
{
    constexpr int skipped = 77;
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_NULL);
    if (!glfwInit())
    {
        std::fprintf(stderr, "gpu tests skipped: GLFW null platform unavailable\n");
        return skipped;
    }
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(16, 16, "gpu tests", nullptr, nullptr);
    if (!window)
    {
        std::fprintf(stderr, "gpu tests skipped: no GL 4.5 context over EGL\n");
        glfwTerminate();
        return skipped;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress) || !gpu::enableDebugOutput())
    {
        std::fprintf(stderr, "gpu tests skipped: no GL 4.5 debug context\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return skipped;
    }
    std::printf("GL: %s, %s\n", (const char *)glGetString(GL_RENDERER), (const char *)glGetString(GL_VERSION));

    doctest::Context context(argc, argv);
    int result = context.run();
    if (!context.shouldExit() && gpu::debugMessageCount() != 0)
    {
        std::fprintf(stderr, "%d unexpected GL debug message(s)\n", gpu::debugMessageCount());
        result = 1;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return result;
}

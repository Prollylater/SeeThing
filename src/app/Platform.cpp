#include "Platform.h"

#include "core/Log.h"
#include "gpu/DebugOutput.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <utility>

Platform::Platform(const char *title, int width, int height, core::Log &log)
{
    // A plain function pointer with no user data, so it cannot reach the log
    glfwSetErrorCallback([](int error, const char *description) { std::fprintf(stderr, "GLFW error %d: %s\n", error, description); });

    // Multi-viewports need window positions, which Wayland forbids: prefer XWayland
    if (glfwPlatformSupported(GLFW_PLATFORM_X11) && std::getenv("DISPLAY"))
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    if (!glfwInit())
        throw std::runtime_error("GLFW could not start (no display?)");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window)
    {
        glfwTerminate();
        throw std::runtime_error("No window with an OpenGL 4.5 core context");
    }
    glfwMakeContextCurrent(m_window);
    if (!gladLoadGL(glfwGetProcAddress))
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
        throw std::runtime_error("Cannot load the OpenGL 4.5 functions");
    }
#ifndef NDEBUG
    if (!gpu::enableDebugOutput(&log))
        log.warn("No GL debug context: GL errors will go unreported");
#endif
    log.info("OpenGL {} on {}", reinterpret_cast<const char *>(glGetString(GL_VERSION)),
             reinterpret_cast<const char *>(glGetString(GL_RENDERER)));

    glfwSetWindowUserPointer(m_window, this);
    glfwSetDropCallback(m_window, [](GLFWwindow *w, int count, const char **paths)
                        {
                            auto *self = static_cast<Platform *>(glfwGetWindowUserPointer(w));
                            for (int i = 0; i < count; ++i)
                                self->m_dropped.emplace_back(paths[i]); });
    glfwSwapInterval(1);
}

Platform::~Platform()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool Platform::shouldClose() const { return glfwWindowShouldClose(m_window); }
void Platform::requestClose() { glfwSetWindowShouldClose(m_window, GLFW_TRUE); }
void Platform::pollEvents() { glfwPollEvents(); }
void Platform::swapBuffers() { glfwSwapBuffers(m_window); }
void Platform::framebufferSize(int &width, int &height) const { glfwGetFramebufferSize(m_window, &width, &height); }
void Platform::setTitle(const std::string &title) { glfwSetWindowTitle(m_window, title.c_str()); }

std::vector<std::string> Platform::takeDroppedPaths()
{
    return std::exchange(m_dropped, {});
}

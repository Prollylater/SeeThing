#ifndef APP_PLATFORM_H
#define APP_PLATFORM_H

#include <string>
#include <vector>

struct GLFWwindow;

namespace core
{
    class Log;
}

// The window and its GL 4.5 context (D2: GLFW for now, kept behind this class)
class Platform
{
public:
    // Throws std::runtime_error when there is no window or no GL 4.5
    Platform(const char *title, int width, int height, core::Log &log);
    ~Platform();
    Platform(const Platform &) = delete;
    Platform &operator=(const Platform &) = delete;

    GLFWwindow *window() const { return m_window; }
    bool shouldClose() const;
    void requestClose();
    void pollEvents();
    void swapBuffers();
    void framebufferSize(int &width, int &height) const;
    void setTitle(const std::string &title);

    // Files dropped on the window since the last call
    std::vector<std::string> takeDroppedPaths();

private:
    GLFWwindow *m_window = nullptr;
    std::vector<std::string> m_dropped;
};

#endif

#ifndef GLCTXT_H
#define GLCTXT_H

#include "glcore.h"

namespace OpenGlContext
{
    void processInput(GLFWwindow *window);

    // Events and input
    void preRender(GLFWwindow *window);
    void postRender(GLFWwindow *window);

    bool initGlContext(GLFWwindow *&window, int width, int height);

    void quit(GLFWwindow *window);
};

#endif


#ifndef GLCTXT_H
#define GLCTXT_H
#include <vector>
#include <iostream>

#include "glcore.h"


namespace OpenGlContext
{
    // TODO
    // Loading pipeline more linked to file browsing   initTextRess(imageress, read_texture<textuc>(1, datapath, matimg));
    void processInput(GLFWwindow * window);

    void preRender();

    void postRender(GLFWwindow *window);


    bool initGlContext(GLFWwindow*& window, int width, int height);

    void quit(GLFWwindow *window);
};

#endif

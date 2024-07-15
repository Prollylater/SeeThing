
#ifndef IMGUICTXT_H
#define IMGUICTXT_H

#include <stdio.h>
#include <vector>

#include "glcore.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Dockspace.h"
#include "ImgEditor.h"
#include "RGPanel.h"

namespace ImGuiAppContext
{

    extern bool showreggrow;
    extern bool showrenderingarea;
   
    void initGlobalStyle();

    // Initialize the Vao allowing to render the image as a texture
    // This vao is always a plane taking a Mat<uint8> data as texture.
    bool init(GLFWwindow*& window);

    void preRender();

    void Render();
    void postRender();

    void quit();
}

#endif

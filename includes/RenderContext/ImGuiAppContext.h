
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

    void initGlobalStyle();

    bool init(GLFWwindow*& window);

    void preRender();

    void Render();
    void postRender(GLFWwindow *window);

    void quit();
}

#endif

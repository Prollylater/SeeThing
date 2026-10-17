#include "ImgApp.h"
#include "AssetPath.h"

#include <stdexcept>

ImgApp::ImgApp() : main_window()
{
  if (!OpenGlContext::initGlContext(main_window, 1024, 576))
    throw std::runtime_error("OpenGL context initialization failed");

  if (!ImGuiAppContext::init(main_window))
    throw std::runtime_error("ImGui initialization failed");

  appobj::glengine = std::make_unique<OpenGLEngine>(assetPath("shaders"));
}

// Swap after ImGui drew; textures released this frame die after that
void ImgApp::start()
{
  while (!glfwWindowShouldClose(main_window))
  {
    OpenGlContext::preRender(main_window);

    ImGuiAppContext::preRender();

    ImGuiAppContext::Render();

    ImGuiAppContext::postRender(main_window);

    OpenGlContext::postRender(main_window);

    appobj::glengine->endFrame();
  }
}

// GL objects go first, while the context is alive
void ImgApp::quit()
{
  appobj::glengine.reset();

  ImGuiAppContext::quit();

  OpenGlContext::quit(main_window);
}

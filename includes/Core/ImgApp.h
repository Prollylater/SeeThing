#include "OpenGLContext.h"
#include "ImGuiAppContext.h"
#include <memory>


class ImgApp
{

private:
  GLFWwindow* main_window;
  OpenglContext Glcontx;

public:
  ImgApp();
  void start();
  void quit();

};

#include "OpenGLContext.h"
#include "ImGuiAppContext.h"


class ImgApp
{

private:
  GLFWwindow* main_window;

public:
  ImgApp();
  void start();
  void quit();

};

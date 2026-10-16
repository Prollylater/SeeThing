#ifndef IMGAPP_H
#define IMGAPP_H
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

#endif

#include "ImgApp.h"

#include <exception>
#include <iostream>

int main(void)
{
  try
  {
    ImgApp app;
    app.start();
    app.quit();
  }
  catch (const std::exception &e)
  {
    std::cerr << "Fatal: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}

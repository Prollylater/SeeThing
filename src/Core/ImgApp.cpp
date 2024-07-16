#include "ImgApp.h"

ImgApp::ImgApp(): main_window() 
{

  
  if (!OpenGlContext::initGlContext(main_window, 1024, 576))
  {
    std::cerr << "OpenGLContext initialization failed" << std::endl;
  }
      if(main_window  == NULL){std::cout<<"stuff"<<std::endl;}

      if(main_window  == nullptr){std::cout<<"stuff"<<std::endl;}

bool eval = ImGuiAppContext::init(main_window);

  if(!eval){
      std::cerr << "ImGuiAppContext initialization failed" << std::endl;
  }

//TODO CREATE ENGINE AND DO STUF

}

void ImgApp::start()
{
  while (!glfwWindowShouldClose(main_window))
  {
    // Render
    
    OpenGlContext::preRender();

    ImGuiAppContext::preRender();

    ImGuiAppContext::Render();

    OpenGlContext::postRender(main_window);

    ImGuiAppContext::postRender();
  }
}
void ImgApp::quit()
{

  // Liberate necessary ressource
  ImGuiAppContext::quit();

  OpenGlContext::quit(main_window);
}

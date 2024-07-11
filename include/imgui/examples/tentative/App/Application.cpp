#include "application.h"


Application::Application(const std::string& app_name)
{
  m_window();
  m_window->init(1024, 720, app_name);

}

void Application::loop()
{
  while (m_window->is_running())
  {
    m_window->render();
  }
}

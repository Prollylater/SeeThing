#include ""
#include ""

class Application
{

public:
  Application(const std::string& app_name);

  static Application& Instance() { return m_instance; }

  void loop();

private:
  static Application m_instance;

  nwindow::GLWindow m_window;
};

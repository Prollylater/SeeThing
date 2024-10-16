#include "glcore.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "UiCompUtils.h"
#include <string>


//Define the main windows where image are rendered
void ShowImageRenderingArea();
void handleClrPencil(ImVec2 windows_pos,ImVec2 windows_size, int img_w, int img_h, ImVec4 color);
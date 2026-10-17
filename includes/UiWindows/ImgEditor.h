#ifndef IMGEDITOR_UI_H
#define IMGEDITOR_UI_H
#include "glcore.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "UiCompUtils.h"
#include "ui/CanvasWidget.h"
#include <string>


//Define the main windows where image are rendered
void ShowImageRenderingArea();
void handleClrPencil(const ui::CanvasInput &input, ImVec4 color);

#endif

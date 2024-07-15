
#ifndef UTILSUI_H
#define UTILSUI_H
#include "glcore.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "IconsCodicons.h" //Simpler here
#include <unordered_map>

void ShowColorPicker(bool &show, ImVec4 &color);

void SetupImGuiFonts();


#endif

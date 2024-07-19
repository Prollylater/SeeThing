
#ifndef UTILSUI_H
#define UTILSUI_H
#include "glcore.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "IconsCodicons.h" //Simpler here
#include <unordered_map>
#include "StructParam.h"

//Simpler here
#include "GlobalStates.h"
#include "ImgEditorEngine.h" //TODO Another place ?


void ShowColorPicker(bool &show, ImVec4 &color);

void SetupImGuiFonts();


//TODO, Not the best position yet again
void SetupGlEngine();



#endif


#ifndef UTILSUI_H
#define UTILSUI_H
#include "glcore.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "IconsCodicons.h" //Simpler here
#include <unordered_map>
#include "StructParam.h"
#include <imfilebrowser.h>

// Simpler here
#include "GlobalStates.h"
#include "ImgEditorEngine.h" //TODO Another place ?

void ShowColorPicker(bool &show, ImVec4 &color);
void ShowGenDisplayPanel(bool &show, GLuint &tex);
// ImGui::Image, flipped vertically, sampled with the texture's own filter
void ImageTexSampler(GLuint tex, const ImVec2 &size);

void SetupImGuiFonts();

#endif

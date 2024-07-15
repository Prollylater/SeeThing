
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
/*
enum class WinStates
{
    img_rendr_area,
    rg_grow_panel,
};
//Class just to have a cosntructor at hand
struct WinStatesHandler{
    std::unordered_map<WinStates,bool> m_states;


    void reverseState(WinStates& state){
        m_states.at(state) =  !m_states.at(state) ;
        resolveDependencies(state);

    }





};
*/


struct RgState{
    
};

#endif

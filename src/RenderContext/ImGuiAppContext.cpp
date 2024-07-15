#include "ImGuiAppContext.h"
#include <iostream>

namespace ImGuiAppContext
{
    bool showreggrow = false;
    bool showrenderingarea = true;

void initGlobalStyle()
{
}

// Initialize the Vao allowing to render the image as a texture
// This vao is always a plane taking a Mat<uint8> data as texture.
bool init(GLFWwindow*& window)
{

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows
    io.ConfigViewportsNoAutoMerge = true;
    io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;              // No rounding
        style.Colors[ImGuiCol_WindowBg].w = 1.0f; // Alpha channel, x, y, z, w
    }

    // Setup Platform/Renderer backends
    const char *glsl_version = "#version 330";
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    //ImVec4 clear_color = ImVec4(0.5f, 0.55f, 0.60f, 1.00f);

    initGlobalStyle();
    SetupImGuiFonts();
    showrenderingarea = true;
    showreggrow = false;
    
    return 1;
}

void preRender()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Create Main docking Area
}
 
bool show_another_window = true;
void Render()
{
    // Take care of each Ui components
    ShowExampleAppDockSpace();
    
    ShowImageRenderingArea(showrenderingarea, showreggrow);
    ShowRegionGrowingArea(showreggrow);
}

void postRender()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO &io = ImGui::GetIO();

    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow *backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void quit()
{ // List of each
    // Loop through this
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
}

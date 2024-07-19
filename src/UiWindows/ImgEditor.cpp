#include "ImgEditor.h"
#include <iostream>

// appobj::canvas
// appobj::glengine
// extern WinStatesHandler win_states;
// extern MainWinStHandler display_states;
// extern RGStatesHandler rg_states;

void ShowImageRenderingArea()
{
    // create a file browser instance
    static ImGui::FileBrowser fileDialog;

    // Temporary value created to allow the use of show
    bool show = win_states.isTrue(WinStates::img_area);
    /*static float f;
         ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
                           ImGui::ColorEdit3("clear color", (float*)&color_pencil);
                             //Push any change to value toward
    win_states.setState(WinStates::img_area,show);
*/

    // Variable tied to this windows
    static ImVec4 color_pencil = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    if (/*!show*/ !win_states.isTrue(WinStates::img_area))
    {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(900, 500), ImGuiCond_FirstUseEver);
    ImGuiWindowFlags window_flags = /*ImGuiWindowFlags_NoTitleBar |*/ ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse;
    // Set the size of the window the first time it appears
    GLuint display;
    int im_width, im_height;

    ImGui::Begin("Edit Mode", nullptr, window_flags);
    {
        //
        // Menu Bar
        if (ImGui::BeginMenuBar())
        { // TODO All bool here are wrong
            // TODO All bool here are wrong
            // TODO All bool here are wrong
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open"))
                {
                    fileDialog.Open();
                }
                if (ImGui::MenuItem("Save"))
                {
                }
                if (ImGui::MenuItem("Save as"))
                {
                }
                if (ImGui::MenuItem("Property"))
                {
                }

                if (ImGui::MenuItem("Close", "Ctrl+W"))
                {
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
        // Menu Bar
        // IMGUI file dialog box
        fileDialog.Display();
        if (fileDialog.HasSelected())
        {
            std::string curr_img= fileDialog.GetSelected().string();
            display_states.setState(MainWinSt::displayed_im, appobj::glengine.ouputImg(curr_img.c_str(), &display, &im_width, &im_height));
            IM_ASSERT(display_states.isTrue(MainWinSt::displayed_im));
        
            fileDialog.ClearSelected();
        }

        //
        //  Calculation regarding center child window

        ImVec2 parent_size = ImGui::GetWindowSize();
        ImVec2 parent_pos = ImGui::GetWindowPos();
        //
        // Icons Bar
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));        // Transparent background
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0)); // Transparent background when hovered
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));

        int count_button = 5;
        ImGui::Dummy(ImVec2((parent_size.x * 0.5f) - count_button * 20.0f, 1.0f));
        ImGui::SameLine();
        const char *icons[] = {ICON_CI_PENCIL, ICON_CI_PAINTCAN, ICON_CI_SEARCH, ICON_CI_SEARCH, ICON_CI_MAP};
        // TODO Write up icon desctipion
        // const char *icons_description[] = {ICON_CI_PENCIL, ICON_CI_PAINTCAN, ICON_CI_SEARCH, ICON_CI_SEARCH, ICON_CI_MAP};

        for (int i = 0; i < count_button; ++i)
        {
            if (i > 0)
            {
                ImGui::SameLine();
            }
            std::string button_label = "Button " + std::to_string(i + 1);
            if (ImGui::Button(icons[i]))
            {
                switch (i)
                {
                case 0:
                    display_states.invState(MainWinSt::clr_pencil);
                    break;
                case 1:
                    break;
                case 2:
                    break;
                case 3:
                    break;
                case 4:
                    win_states.invState(WinStates::rg_panel);
                    break;
                default:
                    break;
                }
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("This is the description for %s", button_label.c_str());
                ImGui::EndTooltip();
            }
        }

        ImGui::PopStyleVar();

        ImGui::PopStyleColor(2);

        ImGui::Separator();
        // Icons Bar
        //
        //  Center window
        ImVec2 child_size(parent_size.x / 1.5, parent_size.y / 1.3);

        ImVec2 child_pos = ImVec2(parent_pos.x + (parent_size.x - child_size.x) * 0.5f,
                                  parent_pos.y + (parent_size.y - child_size.y) * 0.5f);
        // TODO Any other way ?
        // Responsivity
        if ((child_pos.y - parent_pos.y) < 75.0f)
        {
            child_pos = ImVec2(child_pos.x, 300.0f);
        }
        if ((child_pos.x - parent_pos.x) < 75.0f)
        {
            child_pos = ImVec2(300.0f, child_pos.y);
        }

        ImGui::SetNextWindowPos(child_pos, ImGuiCond_Always);
        // Define some window flags
        ImGuiWindowFlags child_window_flags = 0;

        ImGui::BeginChild("ImgHolger", child_size, true, child_window_flags);
        if (display_states.isTrue(MainWinSt::displayed_im))
        {
            ImGui::Image((void *)(intptr_t)display, ImVec2(im_width, im_height));
        }
        if (ImGui::IsWindowHovered())
        {
            ImGui::Text("Hovering over Child Window!");
            ImVec2 widgetPos = ImGui::GetItemRectMin(); // Get position of the current widget
                                                        /* if (ImGui::IsMouseHoveringRect(widgetPos, ImVec2(widgetPos.x + 100, widgetPos.y + 20))) {
                                             }*/
            ImGui::Text("%f %f", widgetPos.x, widgetPos.y);
        }
        ImGui::EndChild();
        // Center window
    }
    ImGui::End();

    bool clr_pen_bool = display_states.isTrue(MainWinSt::clr_pencil);

    ShowColorPicker(clr_pen_bool, color_pencil);
}

/*




*/
#include "ImgEditor.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

// appobj::canvas
// appobj::glengine
// extern WinStatesHandler win_states;
// extern MainWinStHandler display_states;
// extern RGStatesHandler rg_states;


void ShowImageRenderingArea()
{
    // create a file browser instance
    static ImGui::FileBrowser fileDialog;
    static ui::CanvasView view;

    // Variable tied to this windows
    static ImVec4 color_pencil = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    if (!win_states.isTrue(WinStates::img_area))
    {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(900, 500), ImGuiCond_FirstUseEver);
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse;

    if (ImGui::Begin("Edit Mode", nullptr, window_flags))
    {
        // Menu Bar
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open"))
                {
                    fileDialog.Open();
                }
                if (ImGui::MenuItem("Save", nullptr, false, appobj::glengine->hasImage()))
                {
                    std::string error;
                    if (!appobj::glengine->saveMain("./main.jpg", &error))
                        std::cerr << error << std::endl;
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

        fileDialog.Display();
        if (fileDialog.HasSelected())
        {
            std::string error;
            if (!appobj::glengine->openImage(fileDialog.GetSelected().string(), &error))
                std::cerr << error << std::endl;
            display_states.setState(MainWinSt::displayed_im, appobj::glengine->hasImage());
            fileDialog.ClearSelected();
        }

        //
        // Icons Bar
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));        // Transparent background
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0)); // Transparent background when hovered
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));

        int count_button = 5;
        ImGui::Dummy(ImVec2((ImGui::GetWindowSize().x * 0.5f) - count_button * 20.0f, 1.0f));
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
            // Two buttons share the same icon label
            ImGui::PushID(i);
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
            ImGui::PopID();
        }

        ImGui::PopStyleVar();

        ImGui::PopStyleColor(2);

        ImGui::Separator();
        if (appobj::glengine->hasImage())
        {
            ui::CanvasInput input = ui::canvasWidget("main", appobj::glengine->mainTexture(), view);
            if (display_states.isTrue(MainWinSt::clr_pencil))
            {
                handleClrPencil(input, color_pencil);
            }
        }
        else
        {
            ImGui::TextDisabled("File > Open loads an image");
        }
    }
    ImGui::End();

    bool clr_pen_bool = display_states.isTrue(MainWinSt::clr_pencil);

    ShowColorPicker(clr_pen_bool, color_pencil);
}

// Fills the pixels between two frames' positions, so a fast drag leaves no gaps
void handleClrPencil(const ui::CanvasInput &input, ImVec4 color)
{
    static core::Vec2f last;
    if (!input.painting)
    {
        return;
    }
    const core::Vec2f pos{std::floor(input.mouse.x), std::floor(input.mouse.y)};
    std::vector<core::Vec2f> points;
    if (input.strokeStarted)
    {
        points.push_back(pos);
    }
    else
    {
        const core::Vec2f delta = pos - last;
        const int steps = int(std::max(std::fabs(delta.x), std::fabs(delta.y)));
        for (int i = 1; i <= steps; ++i)
        {
            const core::Vec2f p = last + delta * (float(i) / steps);
            points.push_back({std::round(p.x), std::round(p.y)});
        }
    }
    last = pos;
    appobj::glengine->paint(points, {color.x, color.y, color.z});
}

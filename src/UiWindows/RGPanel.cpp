#include "RGPanel.h"
#include "StructParam.h"


void ShowSlicParameter(bool &show)
{
    if (!show)
    {
        return;
    }
    // static float f1, f2, f3;
    static int i1, i2, i3;

    ImGui::SliderInt("CenterPixel\nNumber K", &i1, 1, 1000);
    ImGui::Spacing();
    ImGui::SliderInt("LabSpace\nDiv Strength", &i2, 1, 100);
    ImGui::Spacing();
    ImGui::SliderInt("Compactness m", &i3, 1, 40);

    // Spatial Coherence

    // TODO Add description
    const char *items[] = {"A2", "B2", "D50_2", "D55_2", "D65_2", "D75_2", "E_2", "F1_2"};
    static int xyz_current_idx = 0;
    const char *combo_preview_value = items[xyz_current_idx];

    if (ImGui::BeginCombo("XYZ_Ref", combo_preview_value, ImGuiComboFlags_WidthFitPreview))
    {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
        {
            const bool is_selected = (xyz_current_idx == n);
            if (ImGui::Selectable(items[n], is_selected))
            {
                xyz_current_idx = n;
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}



void ShowSeedParameter(bool &show)
{
    // TODO Provide description POpup of each parameter
    // TODO Execute RegionGrowing Global or with selection
    static bool random = true;
    static float f1, f2, f3;

    
    if (!show)
    {
        return;
    }
    ImGui::Checkbox("Randomize Seeding", &random);

    ImGui::SliderFloat("SeededRegParam1", &f1, 0.0f, 1.0f);
    ImGui::SliderFloat("SeededRegParam2", &f2, 0.0f, 1.0f);

    static ImGuiComboFlags flags = 0;
    /*
         ImGui::CheckboxFlags("ImGuiComboFlags_WidthFitPreview", &flags, ImGuiComboFlags_WidthFitPreview))
         flags &= ~ImGuiComboFlags_NoPreview;

 */

    const char *items[] = {"AAAA", "BBBB", "CCCC", "DDDD", "EEEE"};
    static int item_current_idx = 0;
    const char *combo_preview_value = items[item_current_idx];

    if (ImGui::BeginCombo("Region Comparison Alg", combo_preview_value, flags))
    {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
        {
            const bool is_selected = (item_current_idx == n);
            if (ImGui::Selectable(items[n], is_selected))
            {
                item_current_idx = n;
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    // Dispatch the Result

    if (ImGui::Button("Execute"))
    {
        // DispatchpRegionGrowing
    };
    ImGui::SameLine();
    if (ImGui::Button("Reset"))
    {
        // DispatchpRegionGrowing
    };
    ImGui::SameLine();
    if (ImGui::Button("Test Optimal\n Parameter"))
    {
        // Dispatchpipeline
    };
}

void ShowRegionGrowingArea(bool &show)
{

    int count_button = 5;
    static int mode = 0;
    if (!show)
    {
        return;
    }
    ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
    ImGuiWindowFlags window_flags = /*ImGuiWindowFlags_NoTitleBar |*/ ImGuiWindowFlags_NoCollapse;
    // Set the size of the window the first time it appears

    if (ImGui::Begin("Segmentation Mode", &show, window_flags))
    {

        // Calculation to center child window

        ImVec2 parent_size = ImGui::GetWindowSize();
        ImVec2 parent_pos = ImGui::GetWindowPos();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();

        // ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));        // Transparent background
        // ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0)); // Transparent background when hovered
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));

        ImGui::Dummy(ImVec2((parent_size.x * 0.5f) - count_button * 20.0f, 1.0f));
        ImGui::SameLine();
        const char *icons[] = {ICON_CI_SEARCH, "Slic", "Seed", ICON_CI_SEARCH, ICON_CI_SEARCH};

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
                    mode = 0;
                    break;
                case 1:
                    // SLIC

                    mode = 1;

                    break;
                case 2:
                    // Seeded RegionGrowing
                    mode = 2;
                    break;
                case 3:
                    break;
                case 4:
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

        // ImGui::PopStyleColor(2);
        // TODO Simplify the code
        ImGui::Separator();
        bool modeb;
        switch (mode)
        {

        case 0:
            modeb = (mode == 0);
            break;
        case 1:
            modeb = (mode == 1);
            // Slic Algorithm Parameter
            // Including the colorspace parameter for labspace
            // The k, the number or retry also that other thing
            ShowSlicParameter(modeb);

            break;
        case 2:
            modeb = (mode == 2);
            // Seed Algorithm Parameter, the algorithm
            //  The number of seed,
            //  The posibility to place seed manually too
            ShowSeedParameter(modeb);
            break;
        case 3:
            modeb = (mode == 3);
            // Nothing return the img to non thing
            break;
        case 4:
            modeb = (mode == 4);
            // Region painting
            break;
        }
    }
    ImGui::End();
}

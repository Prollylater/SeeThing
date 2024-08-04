#include "RGPanel.h"

// TODO replace all variables by constant use of their respecitve struct param

// TODO Add usage of active texture and layer concept
// TODO Put description where it's necessary
void ShowSlicParameter(bool &show, GLuint &tex)
{
    if (!show)
    {
        return;
    }
    // Parameter for Slic algorithm
    static int c_pix_k = 1;
    static int lah_div_str = 1;
    static int compactness_m = 1;
    static bool div = false;
    static int coloration_current_idx = 0;

    ImGui::SliderInt("CenterPixel\nNumber K", &c_pix_k, 1, 1000);
    ImGui::Spacing();
    ImGui::SliderInt("LabSpace\nDiv Strength", &lah_div_str, 1, 100);
    ImGui::Spacing();
    ImGui::SliderInt("Compactness m", &compactness_m, 1, 40);

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

    ImGui::SameLine();
    ImGui::Checkbox("Default Div", &div);

    // Dispatch the Result
    if (ImGui::Button("Execute"))
    {
        SlicParameter executeparam(c_pix_k, compactness_m, lah_div_str, xyz_current_idx,
                                   div);
        // TODO GEt active layver
        Mat<uint8_t> &activeim = appobj::canvas.getLayer(0).getImage();

        std::cout << c_pix_k << std::endl;
        std::cout << compactness_m << std::endl;
        std::cout << lah_div_str << std::endl;
        std::cout << div << std::endl;

        Mat<uint8_t>
            regiongrow = appobj::rgengine.dispatch(executeparam, activeim, coloration_current_idx);
        // TODO Dispatch should take care of this ?
        appobj::glengine.outputText(regiongrow, &tex);

        // DispatchpRegionGrowing
    };
    ImGui::SameLine();
    if (ImGui::Button("Default Param"))
    {
        c_pix_k = 100;
        lah_div_str = 1;
        compactness_m = 20;
        xyz_current_idx = 0;
        coloration_current_idx = 0;
    };

    const char *coloration[] = {"Random color", "Average color", "Mean value color"};
    const char *combo_preview_coloration = coloration[coloration_current_idx];

    if (ImGui::BeginCombo("Output Coloration Values", combo_preview_coloration, ImGuiComboFlags_WidthFitPreview))
    {
        for (int n = 0; n < IM_ARRAYSIZE(coloration); n++)
        {
            const bool is_selected = (coloration_current_idx == n);
            if (ImGui::Selectable(coloration[n], is_selected))
            {
                coloration_current_idx = n;
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

void ShowSeedParameter(bool &show, GLuint &tex)
{
    // TODO Provide description POpup of each parameter
    // TODO Execute RegionGrowing Global or with selection
    static bool random = true;
    static bool divide = false;
    static bool adaptive = false;

    static int seednb = 1;
    static float threshold = 0.f;
    static float adapt_perc = 0.f;

    static int coloration_current_idx = 0;

    if (!show)
    {
        return;
    }

    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.4f, 1.0f), "Seed selection parameters");
    ImGui::SliderInt("Seed quantities", &seednb, 1, 5000);
    ImGui::Checkbox("Randomize Seeding", &random);
    ImGui::SameLine();
    if (ImGui::Checkbox("Screen division seeding", &divide))
    {
        random = true;
    };
    // TODO Put a disclaimer

    static ImGuiComboFlags flags = 0;
    /*
         ImGui::CheckboxFlags("ImGuiComboFlags_WidthFitPreview", &flags, ImGuiComboFlags_WidthFitPreview))
         flags &= ~ImGuiComboFlags_NoPreview;

 */
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.4f, 1.0f), "Comparison Algorithm");
    const char *comp_func[] = {"Seed's color Intensity", "Seed's Euclidian distance",
                               "Region's color intensity", "Region's luminance"};
    static int funct_current_idx = 0;
    const char *combo_preview_value = comp_func[funct_current_idx];

    if (ImGui::BeginCombo("Region Comparison Alg", combo_preview_value, flags))
    {
        for (int n = 0; n < IM_ARRAYSIZE(comp_func); n++)
        {
            const bool is_selected = (funct_current_idx == n);
            if (ImGui::Selectable(comp_func[n], is_selected))
            {
                funct_current_idx = n;
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    if (ImGui::Checkbox("Adapative RG mode", &adaptive))
    {
        ImGui::SliderFloat("Adaptive Percentage", &adapt_perc, 0.0f, 100.0f);
    };

    ImGui::SliderFloat("Comparison Threshold", &threshold, 0.0f, 50.0f);

    // Dispatch the Result

    if (ImGui::Button("Execute"))
    {
        // DispatchpRegionGrowing
        std::cout << "sdsd" << std::endl;

        SeedRGParameter executeparam(threshold, adapt_perc, seednb, funct_current_idx, random, divide, adaptive);
        // TODO GEt active layver
        Mat<uint8_t> &activeim = appobj::canvas.getLayer(0).getImage();

        Mat<uint8_t>
            regiongrow = appobj::rgengine.dispatch(executeparam, activeim, coloration_current_idx);
        // TODO Dispatch should take care of this ?
        appobj::glengine.outputText(regiongrow, &tex);
    };
    ImGui::SameLine();
    if (ImGui::Button("Default Param"))
    {
        // DispatchpRegionGrowing
        adapt_perc = 50.0f;
        threshold = 15.0f;
    };
    ImGui::SameLine();
    if (ImGui::Button("Test Optimal\n Parameter"))
    {
        // Dispatchpipeline
    };

    const char *coloration[] = {"Random color", "Average color", "Mean value color"};
    const char *combo_preview_coloration = coloration[coloration_current_idx];

    if (ImGui::BeginCombo("Output Coloration Values", combo_preview_coloration, ImGuiComboFlags_WidthFitPreview))
    {
        for (int n = 0; n < IM_ARRAYSIZE(coloration); n++)
        {
            const bool is_selected = (coloration_current_idx == n);
            if (ImGui::Selectable(coloration[n], is_selected))
            {
                coloration_current_idx = n;
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

// Result display may need better check or tto be removed
//  TODO The boolean system fro selection is weird
void ShowRegionGrowingArea()
{
    static GLuint display;

    bool show = win_states.isTrue(WinStates::rg_panel);

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
        static bool result_display = false;

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
        const char *icons[] = {ICON_CI_SEARCH, "Slic", "Seed", ICON_CI_SEARCH, ICON_CI_PRESERVE_CASE};
        // const char *desctiptions[] = {ICON_CI_SEARCH, "Slic", "Seed", ICON_CI_SEARCH, ICON_CI_SEARCH};

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
                    mode = 4;
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
            ShowSlicParameter(modeb, display);

            break;
        case 2:
            modeb = (mode == 2);
            // Seed Algorithm Parameter, the algorithm
            //  The number of seed,
            //  The posibility to place seed manually too
            ShowSeedParameter(modeb, display);
            break;
        case 3:
            modeb = (mode == 3);
            // Region painting

            break;
        case 4:
            modeb = (mode == 4);
            // Nothing return the img to non thing

            break;
        }

        if (/*width * height != 0 &&*/ display != 0)
        {
            result_display = true;
            ShowGenDisplayPanel(result_display, display);
        }
    }
    ImGui::End();
}
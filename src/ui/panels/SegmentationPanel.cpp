#include "SegmentationPanel.h"

#include "app/App.h"

#include "imgui.h"

namespace ui
{
    namespace
    {
        const char *colorations[] = {"Random colour per region", "Average colour of the region", "Mean intensity of the region"};

        bool combo(const char *label, int &index, const char *const *items, int count)
        {
            bool changed = false;
            if (ImGui::BeginCombo(label, items[index]))
            {
                for (int n = 0; n < count; ++n)
                {
                    const bool selected = index == n;
                    if (ImGui::Selectable(items[n], selected))
                    {
                        index = n;
                        changed = true;
                    }
                    if (selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            return changed;
        }

        // Disabled items show no tooltip unless asked
        void disabledTooltip(const char *text)
        {
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                ImGui::SetTooltip("%s", text);
        }

        bool runButton(const App &app)
        {
            ImGui::BeginDisabled(!app.hasImage());
            const bool clicked = ImGui::Button("Run");
            ImGui::EndDisabled();
            disabledTooltip(app.hasImage() ? "Runs on the UI thread: the window waits until it finishes" : "Open an image first");
            return clicked;
        }
    }

    void SegmentationPanel::draw(App &app, bool &open)
    {
        if (!open)
            return;
        ImGui::SetNextWindowSize(ImVec2(340, 520), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(name, &open) && ImGui::BeginTabBar("##algorithms"))
        {
            if (ImGui::BeginTabItem("SLIC"))
            {
                drawSlic(app);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Seeded growing"))
            {
                drawSeeded(app);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }

    void SegmentationPanel::drawSlic(App &app)
    {
        Slic &s = m_slic;
        ImGui::TextWrapped("Superpixels: K clusters seeded on a grid, grown in Lab colour and position.");
        ImGui::Spacing();
        ImGui::SliderInt("Superpixels K", &s.k, 1, 1000);
        ImGui::SetItemTooltip("Number of clusters. The grid step is sqrt(pixels / K)");
        ImGui::SliderInt("Compactness m", &s.compactness, 1, 40);
        ImGui::SetItemTooltip("Weight of distance against colour: higher gives rounder, more regular superpixels");

        // Same order as XYZRefHandle and XYZrefs in Mat.h
        const char *illuminants[] = {"A (tungsten)", "B (old noon sun)", "C (old daylight)", "D50 (ICC)", "D55 (morning)",
                                     "D65 (sRGB daylight)", "D75 (north sky)", "E (equal energy)", "F1 (fluorescent)"};
        combo("Reference white", s.illuminant, illuminants, IM_COUNTOF(illuminants));
        ImGui::SetItemTooltip("White point used to normalise XYZ before Lab. D65 matches sRGB images");
        ImGui::Checkbox("Scale XYZ", &s.scaleXyz);
        ImGui::SetItemTooltip("Multiplies XYZ by the factor below before Lab; off is standard Lab");
        if (s.scaleXyz)
            ImGui::SliderInt("Factor", &s.xyzScale, 1, 100);

        combo("Output", s.coloration, colorations, IM_COUNTOF(colorations));
        ImGui::Spacing();
        if (runButton(app))
            app.runSlic(SlicParameter(s.k, s.compactness, s.xyzScale, s.illuminant, s.scaleXyz), s.coloration);
        ImGui::SameLine();
        if (ImGui::Button("Defaults"))
            s = Slic{};
    }

    void SegmentationPanel::drawSeeded(App &app)
    {
        Seeded &s = m_seeded;
        ImGui::TextWrapped("Seeds grow into regions while neighbours stay under the threshold; close regions are merged.");
        ImGui::Spacing();
        ImGui::SliderInt("Seeds", &s.seeds, 1, 5000);
        ImGui::BeginDisabled();
        ImGui::Checkbox("Random seeds", &s.random);
        ImGui::EndDisabled();
        disabledTooltip("Placing seeds by hand comes with the seed tool (P6)");
        ImGui::SameLine();
        ImGui::Checkbox("One per grid cell", &s.divide);
        ImGui::SetItemTooltip("Spreads the seeds on a grid instead of anywhere in the image");

        // Same order as rgs_comp_func_var in RegionGrowingAlg.h
        const char *compare[] = {"Seed colour", "Region colour", "Region luminance", "Distance to seed colour"};
        combo("Compare with", s.compare, compare, IM_COUNTOF(compare));
        ImGui::SetItemTooltip("What a neighbour pixel is compared to before it joins a region");
        ImGui::SliderFloat("Threshold", &s.threshold, 0.0f, 50.0f, "%.1f");
        ImGui::SetItemTooltip("Largest difference that still joins the region; also the merge threshold");
        ImGui::Checkbox("Adaptive", &s.adaptive);
        ImGui::SetItemTooltip("Compare with the current pixel until the share below of the image is visited, then with the region's colour. Ignores \"Compare with\"");
        if (s.adaptive)
        {
            ImGui::SliderFloat("Switch at", &s.adaptPercent, 0.0f, 100.0f, "%.0f%% visited");
            ImGui::SetItemTooltip("Share of the image visited before comparisons switch to the region's colour");
        }

        combo("Output", s.coloration, colorations, IM_COUNTOF(colorations));
        ImGui::Spacing();
        if (runButton(app))
            app.runSeeded(SeedRGParameter(s.threshold, s.adaptPercent, s.seeds, s.compare, s.random, s.divide, s.adaptive), s.coloration);
        ImGui::SameLine();
        if (ImGui::Button("Defaults"))
            s = Seeded{};
    }
}

#include "SegmentationPanel.h"

#include "app/App.h"
#include "ui/tools/Tool.h"

#include "imgui.h"

namespace ui
{
    namespace
    {
        const char *const colorations[] = {"Random colour per region", "Average colour of the region", "Mean intensity of the region"};

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
        Slic &slic = m_slic;
        ImGui::TextWrapped("Superpixels: K clusters seeded on a grid, grown in Lab colour and position.");
        ImGui::Spacing();
        ImGui::SliderInt("Superpixels K", &slic.k, 1, 1000);
        ImGui::SetItemTooltip("Number of clusters. The grid step is sqrt(pixels / K)");
        ImGui::SliderInt("Compactness m", &slic.compactness, 1, 40);
        ImGui::SetItemTooltip("Weight of distance against colour: higher gives rounder, more regular superpixels");

        // Same order as XYZRefHandle and XYZrefs in Mat.h
        const char *illuminants[] = {"A (tungsten)", "B (old noon sun)", "C (old daylight)", "D50 (ICC)", "D55 (morning)",
                                     "D65 (sRGB daylight)", "D75 (north sky)", "E (equal energy)", "F1 (fluorescent)"};
        combo("Reference white", slic.illuminant, illuminants, IM_COUNTOF(illuminants));
        ImGui::SetItemTooltip("White point used to normalise XYZ before Lab. D65 matches sRGB images");
        ImGui::Checkbox("Scale XYZ", &slic.scaleXyz);
        ImGui::SetItemTooltip("Multiplies XYZ by the factor below before Lab; off is standard Lab");
        if (slic.scaleXyz)
            ImGui::SliderInt("Factor", &slic.xyzScale, 1, 100);

        combo("Output", slic.coloration, colorations, IM_COUNTOF(colorations));
        ImGui::Spacing();
        if (runButton(app))
            app.runSlic(SlicParameter(slic.k, slic.compactness, slic.xyzScale, slic.illuminant, slic.scaleXyz), slic.coloration);
        ImGui::SameLine();
        if (ImGui::Button("Defaults"))
            slic = Slic{};
    }

    void SegmentationPanel::drawSeeded(App &app)
    {
        Seeded &seeded = m_seeded;
        ImGui::TextWrapped("Seeds grow into regions while neighbours stay under the threshold; close regions are merged.");
        ImGui::Spacing();
        const char *sources[] = {"Random", "One per grid cell", "Placed with the Seeds tool"};
        combo("Seeds from", seeded.source, sources, IM_COUNTOF(sources));
        ImGui::SetItemTooltip("Random: anywhere. Grid: spread evenly. Placed: the seeds on the image (Seeds tool, S)");
        const bool placed = seeded.source == 2;
        if (placed)
            ImGui::Text("%zu seeds placed", app.document().seeds().size());
        else
            ImGui::SliderInt("Seeds", &seeded.seeds, 1, 5000);

        combo("Compare with", seeded.compare, compareNames, IM_COUNTOF(compareNames));
        ImGui::SetItemTooltip("What a neighbour pixel is compared to before it joins a region");
        ImGui::SliderFloat("Threshold", &seeded.threshold, 0.0f, 50.0f, "%.1f");
        ImGui::SetItemTooltip("Largest difference that still joins the region; also the merge threshold");
        ImGui::Checkbox("Adaptive", &seeded.adaptive);
        ImGui::SetItemTooltip("Compare with the current pixel until the share below of the image is visited, then with the region's colour. Ignores \"Compare with\"");
        if (seeded.adaptive)
        {
            ImGui::SliderFloat("Switch at", &seeded.adaptPercent, 0.0f, 100.0f, "%.0f%% visited");
            ImGui::SetItemTooltip("Share of the image visited before comparisons switch to the region's colour");
        }

        combo("Output", seeded.coloration, colorations, IM_COUNTOF(colorations));
        ImGui::Spacing();
        if (runButton(app))
            app.runSeeded(SeedRGParameter(seeded.threshold, seeded.adaptPercent, seeded.seeds, seeded.compare, !placed, seeded.source == 1, seeded.adaptive), seeded.coloration);
        ImGui::SameLine();
        if (ImGui::Button("Defaults"))
            seeded = Seeded{};
    }
}

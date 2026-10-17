#include "CanvasPanel.h"

#include "IconsCodicons.h"
#include "app/App.h"
#include "ui/Shell.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace ui
{
    namespace
    {
        // A toolbar button drawn pressed while its state is on
        bool toggleButton(const char *label, bool on, const char *tooltip)
        {
            if (on)
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
            const bool clicked = ImGui::Button(label);
            if (on)
                ImGui::PopStyleColor();
            ImGui::SetItemTooltip("%s", tooltip);
            return clicked;
        }
    }

    void CanvasPanel::draw(App &app, Shell &shell, bool &open)
    {
        if (!open)
            return;
        ImGui::SetNextWindowSize(ImVec2(900, 600), ImGuiCond_FirstUseEver);
        // The canvas takes the wheel and fills the window: nothing to scroll
        if (!ImGui::Begin(name, &open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            ImGui::End();
            return;
        }

        if (toggleButton(ICON_CI_EDIT "##pencil", m_pencil, "Pencil: left drag paints on the image"))
            m_pencil = !m_pencil;
        if (m_pencil)
        {
            ImGui::SameLine();
            ImGui::ColorEdit3("##pencil_color", &m_color.x, ImGuiColorEditFlags_NoInputs);
            ImGui::SetItemTooltip("Pencil colour");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(120.0f);
            ImGui::SliderInt("Size", &m_size, 1, 32, "%d px");
        }
        ImGui::SameLine();
        bool &segmentation = shell.segmentationOpen();
        if (toggleButton(ICON_CI_TYPE_HIERARCHY "##segmentation", segmentation, "Segmentation panel: SLIC and seeded region growing"))
            segmentation = !segmentation;
        ImGui::Separator();

        if (app.hasImage())
        {
            const CanvasInput input = canvasWidget("main", app.gpu().mainTexture(), m_view);
            if (m_pencil)
                paint(app, input);
        }
        else
        {
            ImGui::Spacing();
            ImGui::TextDisabled("No image. Open one, or drop a file on the window.");
            if (ImGui::Button("Open image..."))
                shell.openImageDialog();
        }
        ImGui::End();
    }

    // Fills the pixels between two frames' positions, so a fast drag leaves no gaps
    void CanvasPanel::paint(App &app, const CanvasInput &input)
    {
        if (!input.painting)
            return;
        const core::Vec2f pos{std::floor(input.mouse.x), std::floor(input.mouse.y)};
        std::vector<core::Vec2f> points;
        if (input.strokeStarted)
        {
            points.push_back(pos);
        }
        else
        {
            const core::Vec2f delta = pos - m_last;
            const int steps = int(std::max(std::fabs(delta.x), std::fabs(delta.y)));
            for (int i = 1; i <= steps; ++i)
            {
                const core::Vec2f p = m_last + delta * (float(i) / steps);
                points.push_back({std::round(p.x), std::round(p.y)});
            }
        }
        m_last = pos;
        app.gpu().paint(points, {m_color.x, m_color.y, m_color.z}, float(m_size));
    }
}

#include "CanvasPanel.h"

#include "IconsCodicons.h"
#include "app/App.h"
#include "ui/SelectionOverlay.h"
#include "ui/Shell.h"
#include "ui/tools/Tools.h"

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

    CanvasPanel::CanvasPanel() : m_tools(makeTools())
    {
    }

    void CanvasPanel::selectTool(App &app, std::size_t index)
    {
        if (index == m_tool || index >= m_tools.size())
            return;
        cancelGesture(app);
        m_tool = index;
    }

    void CanvasPanel::cancelGesture(App &app)
    {
        if (!m_gesture)
            return;
        ToolContext ctx{app, m_shared};
        m_tools[m_tool]->cancel(ctx);
        m_gesture = false;
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

        drawToolbox(app, shell);
        if (app.hasImage())
        {
            OpenGLEngine &gpu = app.gpu();
            Tool &tool = *m_tools[m_tool];
            const CanvasInput input = canvasWidget("main", gpu.compositeTexture(), m_view, tool.leftDragPans());
            if (app.document().hasSelection())
                drawSelectionOverlay(gpu.renderer(), gpu.selectionTexture(), m_view, input);
            handleInput(app, input);

            ImDrawList *dl = ImGui::GetWindowDrawList();
            const core::Vec2f end = input.origin + input.size;
            dl->PushClipRect({input.origin.x, input.origin.y}, {end.x, end.y}, true);
            drawSeeds(app, input);
            tool.overlay(dl, m_view, input);
            dl->PopClipRect();
        }
        else
        {
            cancelGesture(app);
            ImGui::Spacing();
            ImGui::TextDisabled("No image. Open one, or drop a file on the window.");
            if (ImGui::Button("Open image..."))
                shell.openImageDialog();
        }
        ImGui::End();
    }

    void CanvasPanel::drawToolbox(App &app, Shell &shell)
    {
        // Single keys, unless a text field or a dialog has the keyboard
        if (!ImGui::GetIO().WantTextInput && !shell.dialogOpen())
            for (std::size_t i = 0; i < m_tools.size(); ++i)
                if (ImGui::Shortcut(m_tools[i]->key(), ImGuiInputFlags_RouteGlobal))
                    selectTool(app, i);

        for (std::size_t i = 0; i < m_tools.size(); ++i)
        {
            ImGui::PushID(int(i));
            if (toggleButton(m_tools[i]->icon(), i == m_tool, m_tools[i]->help()))
                selectTool(app, i);
            ImGui::PopID();
            ImGui::SameLine();
        }
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        bool &segmentation = shell.segmentationOpen();
        if (toggleButton(ICON_CI_TYPE_HIERARCHY "##segmentation", segmentation, "Segmentation panel: SLIC and seeded region growing"))
            segmentation = !segmentation;

        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(m_tools[m_tool]->name());
        ImGui::SameLine();
        ToolContext ctx{app, m_shared};
        ImGui::BeginDisabled(!app.hasImage());
        m_tools[m_tool]->options(ctx);
        ImGui::EndDisabled();
        ImGui::Separator();
    }

    // The canvas reports a left-button gesture; the active tool gets it in image coordinates
    void CanvasPanel::handleInput(App &app, const CanvasInput &input)
    {
        Tool &tool = *m_tools[m_tool];
        ToolContext ctx{app, m_shared};
        const ImGuiIO &io = ImGui::GetIO();
        const ToolEvent e{input.mouse, m_view.zoom, io.KeyShift, io.KeyAlt};
        if (input.strokeStarted)
        {
            m_gesture = true;
            tool.press(ctx, e);
        }
        else if (m_gesture && input.painting)
        {
            tool.drag(ctx, e);
        }
        if (m_gesture && input.strokeEnded)
        {
            m_gesture = false;
            tool.release(ctx, e);
        }
    }

    void CanvasPanel::drawSeeds(const App &app, const CanvasInput &input) const
    {
        ImDrawList *dl = ImGui::GetWindowDrawList();
        for (core::Vec2i s : app.document().seeds())
        {
            const core::Vec2f c = m_view.toScreen({s.x + 0.5f, s.y + 0.5f}, input.origin);
            dl->AddCircleFilled({c.x, c.y}, 5.0f, IM_COL32(0, 0, 0, 200));
            dl->AddCircleFilled({c.x, c.y}, 3.5f, IM_COL32(60, 230, 120, 255));
        }
    }
}

#ifndef UI_TOOL_H
#define UI_TOOL_H

#include "ui/CanvasView.h"
#include "ui/CanvasWidget.h"

#include "imgui.h"

class App;

namespace ui
{
    // Same order as rgs_comp_func_var in RegionGrowingAlg.h
    inline constexpr const char *compareNames[] = {"Seed colour", "Region colour", "Region luminance", "Distance to seed colour"};

    // What the tools share: the foreground colour
    struct ToolShared
    {
        ImVec4 color{0.90f, 0.20f, 0.20f, 1.0f};
    };

    struct ToolContext
    {
        App &app;
        ToolShared &shared;
    };

    // One pointer event, in image coordinates
    struct ToolEvent
    {
        core::Vec2f pos;
        float zoom = 1.0f;
        bool shift = false, alt = false;
        // 1 until a platform with pen input feeds it (SDL3, P8)
        float pressure = 1.0f;
    };

    // A canvas tool: a left-button gesture press, drag..., release, plus options and screen-space feedback.
    class Tool
    {
    public:
        virtual ~Tool() = default;
        virtual const char *name() const = 0;
        virtual const char *icon() const = 0;
        virtual const char *help() const = 0;
        virtual ImGuiKey key() const = 0;

        // Drawn on the toolbar row below the toolbox while the tool is active
        virtual void options(ToolContext &) {}
        virtual void press(ToolContext &, const ToolEvent &) {}
        virtual void drag(ToolContext &, const ToolEvent &) {}
        virtual void release(ToolContext &, const ToolEvent &) {}
        // The gesture stops early: another tool was chosen or the image went away
        virtual void cancel(ToolContext &) {}
        virtual void overlay(ImDrawList *, const CanvasView &, const CanvasInput &) const {}
        virtual bool leftDragPans() const { return false; }
    };
}

#endif

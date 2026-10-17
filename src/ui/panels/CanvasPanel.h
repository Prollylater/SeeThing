#ifndef UI_CANVASPANEL_H
#define UI_CANVASPANEL_H

#include "ui/CanvasView.h"
#include "ui/CanvasWidget.h"
#include "ui/tools/Tool.h"

#include <memory>
#include <vector>

class App;

namespace ui
{
    class Shell;

    // The composite of the visible layers, the toolbox and the active tool's options
    class CanvasPanel
    {
    public:
        static constexpr const char *name = "Image";

        CanvasPanel();
        void draw(App &app, Shell &shell, bool &open);
        CanvasView &view() { return m_view; }
        // Ends a gesture in progress first
        void selectTool(App &app, std::size_t index);

    private:
        void cancelGesture(App &app);
        void drawToolbox(App &app, Shell &shell);
        void handleInput(App &app, const CanvasInput &input);
        void drawSeeds(const App &app, const CanvasInput &input) const;

        CanvasView m_view;
        ToolShared m_shared;
        std::vector<std::unique_ptr<Tool>> m_tools;
        std::size_t m_tool = 0;
        bool m_gesture = false;
    };
}

#endif

#ifndef UI_CANVASPANEL_H
#define UI_CANVASPANEL_H

#include "ui/CanvasView.h"
#include "ui/CanvasWidget.h"

#include "imgui.h"

class App;

namespace ui
{
    class Shell;

    // The main image with the pencil
    class CanvasPanel
    {
    public:
        static constexpr const char *name = "Image";

        void draw(App &app, Shell &shell, bool &open);
        CanvasView &view() { return m_view; }

    private:
        void paint(App &app, const CanvasInput &input);

        CanvasView m_view;
        bool m_pencil = false;
        ImVec4 m_color{0.90f, 0.20f, 0.20f, 1.0f};
        int m_size = 2;
        core::Vec2f m_last;
    };
}

#endif

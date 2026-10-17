#ifndef UI_RESULTPANEL_H
#define UI_RESULTPANEL_H

#include "ui/CanvasView.h"

class App;

namespace ui
{
    class Shell;

    class ResultPanel
    {
    public:
        static constexpr const char *name = "Result";

        void draw(App &app, Shell &shell);

    private:
        CanvasView m_view;
    };
}

#endif

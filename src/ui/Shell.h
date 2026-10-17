#ifndef UI_SHELL_H
#define UI_SHELL_H

#include "panels/CanvasPanel.h"
#include "panels/HistoryPanel.h"
#include "panels/LayersPanel.h"
#include "panels/LogPanel.h"
#include "panels/ResultPanel.h"
#include "panels/SegmentationPanel.h"

#include "imgui.h"
#include <imfilebrowser.h>

class App;
class Platform;

namespace ui
{
    // ImGui, the menu bar, the dock layout, the dialogs, and the panels with their open flags
    class Shell
    {
    public:
        Shell(App &app, Platform &platform);
        ~Shell();
        Shell(const Shell &) = delete;
        Shell &operator=(const Shell &) = delete;

        void beginFrame();
        void draw();
        // ImGui::Render, then draws into the cleared default framebuffer and the other viewports
        void render(Platform &platform);

        void openImageDialog();
        // Save As when nothing was saved yet, so the opened file is never overwritten by surprise
        void save();
        void saveImageDialog();
        void saveResultDialog();
        bool &segmentationOpen() { return m_showSegmentation; }
        bool dialogOpen() const { return m_openDialog.IsOpened() || m_saveDialog.IsOpened(); }

    private:
        enum class SaveTarget
        {
            Image,
            Result,
        };

        void drawMenuBar();
        void handleShortcuts();
        void drawDialogs();
        void drawControls();
        void buildDefaultLayout(ImGuiID dockspace);

        App &m_app;
        CanvasPanel m_canvas;
        SegmentationPanel m_segmentation;
        ResultPanel m_result;
        LogPanel m_logPanel;
        LayersPanel m_layers;
        HistoryPanel m_history;
        bool m_showImage = true, m_showSegmentation = true, m_showLog = true, m_showLayers = true, m_showHistory = true;
        bool m_showControls = false;
        bool m_resetLayout = false;
        ImGui::FileBrowser m_openDialog;
        ImGui::FileBrowser m_saveDialog;
        SaveTarget m_saveTarget = SaveTarget::Image;
    };
}

#endif

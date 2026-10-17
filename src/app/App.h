#ifndef APP_APP_H
#define APP_APP_H

#include "Platform.h"
#include "core/Log.h"
#include "ui/Shell.h"

#include "Canvas.h"
#include "ImgEditorEngine.h"
#include "RegionGrowingEngine.h"
#include "StructParam.h"

#include <string>

// Owns everything. Member order is lifetime order: the log outlives all that report to it,
// and every GL owner is destroyed while the platform's context is still alive.
class App
{
public:
    App();
    void run();

    bool openImage(const std::string &path);
    // Writes the displayed image, pencil strokes included
    bool saveImage(const std::string &path);
    void closeImage();
    bool hasImage() const { return m_gpu.hasImage(); }
    // The file the image came from, and the one Save writes (empty until a Save As)
    const std::string &imagePath() const { return m_image_path; }
    const std::string &savePath() const { return m_save_path; }

    void runSlic(const SlicParameter &param, int coloration);
    void runSeeded(const SeedRGParameter &param, int coloration);
    bool saveResult(const std::string &path);
    void quit() { m_platform.requestClose(); }

    core::Log &log() { return m_log; }
    OpenGLEngine &gpu() { return m_gpu; }
    Canvas &canvas() { return m_canvas; }

private:
    void updateTitle();

    core::Log m_log;
    Platform m_platform;
    OpenGLEngine m_gpu;
    Canvas m_canvas;
    RGEngine m_rg;
    std::string m_image_path, m_save_path;
    ui::Shell m_shell;
};

#endif

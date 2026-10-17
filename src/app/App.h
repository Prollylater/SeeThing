#ifndef APP_APP_H
#define APP_APP_H

#include "Platform.h"
#include "core/Log.h"
#include "doc/Document.h"
#include "doc/Selection.h"
#include "ui/Shell.h"

#include "ImgEditorEngine.h"
#include "RegionGrowingEngine.h"
#include "StructParam.h"

#include <array>
#include <functional>
#include <optional>
#include <span>
#include <string>

// Owns everything. Member order is lifetime order: the log outlives all that report to it,
// and every GL owner is destroyed while the platform's context is still alive.
class App
{
public:
    App();
    void run();

    bool openImage(const std::string &path);
    // Writes the composite of the visible layers
    bool saveImage(const std::string &path);
    void closeImage();
    bool hasImage() const { return !m_doc.empty(); }
    // The file the image came from, and the one Save writes (empty until a Save As)
    const std::string &imagePath() const { return m_image_path; }
    const std::string &savePath() const { return m_save_path; }

    doc::Document &document() { return m_doc; }
    const doc::Document &document() const { return m_doc; }
    void undo();
    void redo();
    void jumpTo(std::size_t cursor);

    // Paints into the active layer; false when it is locked or hidden
    bool beginStroke();
    void strokePoints(std::span<const core::Vec2f> pixels, core::Vec3f color, float size);
    // Brush or eraser on the active layer; centres in pixel coordinates
    bool beginStamp(OpenGLEngine::StampStyle style);
    void stampDabs(std::span<const core::Vec2f> centers, float diameter, float hardness, float flow);
    // Copies the stroke into the layer and records it as one step
    void endStroke();

    // The pixel of the composite (what you see) or of the active layer
    std::optional<std::array<std::uint8_t, 4>> pickColor(core::Vec2i p, bool composite);
    // Both grow one region from p on the active layer, with the seeded growing's comparison.
    // The bucket stays inside the selection; the wand doesn't, so it can add regions outside it.
    bool bucketFill(core::Vec2i p, float threshold, int compare, std::array<std::uint8_t, 4> rgba);
    bool magicWand(core::Vec2i p, float threshold, int compare, doc::SelectMode mode);

    // Both read the active layer, limited to the selection when there is one; seeded growing in Placed mode
    // starts from the document's seeds
    void runSlic(const SlicParameter &param, int coloration);
    void runSeeded(const SeedRGParameter &param, int coloration);
    bool hasResult() const { return !m_result.empty(); }
    const std::string &resultLabel() const { return m_result_label; }
    bool saveResult(const std::string &path);
    void discardResult();
    // Above the layer the algorithm read, as a Result layer
    void addResultAsLayer();
    // Replaces the active layer's pixels
    void copyResultToLayer();

    void quit() { m_platform.requestClose(); }

    core::Log &log() { return m_log; }
    OpenGLEngine &gpu() { return m_gpu; }

private:
    void updateTitle();
    void setResult(core::Image<std::uint8_t> image, std::string label, std::string origin);
    core::Image<std::uint8_t> activePixels() const;
    // The active layer when it can take paint; logs why not otherwise
    const doc::Layer *paintableLayer();
    // Runs on activePixels() with the selection as the region of interest
    core::Image<std::uint8_t> runOnActive(const std::function<Mat<uint8_t>(const Mat<uint8_t> &, const Mat<uint8_t> *)> &run, double &ms);

    core::Log m_log;
    Platform m_platform;
    OpenGLEngine m_gpu;
    doc::Document m_doc;
    RGEngine m_rg;
    std::string m_image_path, m_save_path;
    core::Image<std::uint8_t> m_result;
    std::string m_result_label, m_result_origin;
    doc::LayerId m_result_source = 0;
    std::string m_stroke_label;
    ui::Shell m_shell;
};

#endif

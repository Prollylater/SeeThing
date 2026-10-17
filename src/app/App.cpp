#include "App.h"

#include "AssetPath.h"
#include "MatBridge.h"
#include "core/Color.h"
#include "core/ImageIO.h"
#include "doc/Edits.h"
#include "RegionFill.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <format>
#include <functional>

namespace
{
    std::string fileName(const std::string &path)
    {
        return std::filesystem::path(path).filename().string();
    }

    core::Image<std::uint8_t> copyOf(core::ImageView<const std::uint8_t> v)
    {
        core::Image<std::uint8_t> out(v.width(), v.height(), v.channels());
        for (int y = 0; y < v.height(); ++y)
            std::copy_n(v.row(y), std::size_t(v.width()) * v.channels(), out.row(y));
        return out;
    }

    template <class F>
    double millisecondsOf(F &&f)
    {
        const auto start = std::chrono::steady_clock::now();
        f();
        return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    }
}

App::App()
    : m_platform("SeeThing", 1280, 800, m_log),
      m_gpu(assetPath("shaders"), m_log),
      m_shell(*this, m_platform)
{
}

void App::run()
{
    while (!m_platform.shouldClose())
    {
        m_platform.pollEvents();
        std::vector<std::string> dropped = m_platform.takeDroppedPaths();
        if (!dropped.empty())
        {
            openImage(dropped.front());
            if (dropped.size() > 1)
                m_log.warn("Dropped {} files; only the first one was opened", dropped.size());
        }

        m_shell.beginFrame();
        m_shell.draw();
        m_gpu.sync(m_doc);
        m_shell.render(m_platform);
        m_platform.swapBuffers();
        m_gpu.endFrame();
    }
}

bool App::openImage(const std::string &path)
{
    core::LoadResult loaded = core::loadImage(path, 0);
    if (!loaded.ok())
    {
        m_log.error("{}", loaded.error);
        return false;
    }
    const int channels = loaded.image.channels();
    m_gpu.endStroke();
    doc::Document doc(loaded.image.width(), loaded.image.height());
    doc.insertLayer(0, doc::Layer(fileName(path), core::toRGBA8(loaded.image)));
    m_doc = std::move(doc);
    discardResult();

    m_image_path = path;
    m_save_path.clear();
    m_log.info("Opened {} ({} x {}, {} channels)", path, m_doc.width(), m_doc.height(), channels);
    updateTitle();
    return true;
}

bool App::saveImage(const std::string &path)
{
    m_gpu.sync(m_doc);
    std::string error;
    if (!m_gpu.saveComposite(path, &error))
    {
        m_log.error("{}", error);
        return false;
    }
    m_save_path = path;
    m_log.info("Saved {}", path);
    updateTitle();
    return true;
}

void App::closeImage()
{
    m_gpu.endStroke();
    m_doc = doc::Document();
    discardResult();
    m_image_path.clear();
    m_save_path.clear();
    updateTitle();
}

void App::undo()
{
    endStroke();
    m_doc.undo();
}

void App::redo()
{
    endStroke();
    m_doc.redo();
}

void App::jumpTo(std::size_t cursor)
{
    endStroke();
    doc::History &h = m_doc.history();
    while (h.cursor() > cursor && h.canUndo())
        m_doc.undo();
    while (h.cursor() < cursor && h.canRedo())
        m_doc.redo();
}

const doc::Layer *App::paintableLayer()
{
    const doc::Layer *layer = m_doc.activeLayer();
    if (layer && (layer->props().locked || !layer->props().visible))
    {
        m_log.warn("{} is {}; nothing painted", layer->props().name, layer->props().locked ? "locked" : "hidden");
        return nullptr;
    }
    return layer;
}

bool App::beginStroke()
{
    const doc::Layer *layer = paintableLayer();
    if (!layer)
        return false;
    endStroke();
    m_gpu.beginStroke(m_doc, layer->id());
    m_stroke_label = "Pencil";
    return true;
}

bool App::beginStamp(OpenGLEngine::StampStyle style)
{
    const doc::Layer *layer = paintableLayer();
    if (!layer)
        return false;
    endStroke();
    m_gpu.beginStamp(m_doc, layer->id(), style);
    m_stroke_label = style.erase ? "Eraser" : "Brush";
    return true;
}

void App::stampDabs(std::span<const core::Vec2f> centers, float diameter, float hardness, float flow)
{
    m_gpu.stampDabs(centers, diameter, hardness, flow);
}

void App::strokePoints(std::span<const core::Vec2f> pixels, core::Vec3f color, float size)
{
    m_gpu.strokePoints(m_doc, pixels, color, size);
}

void App::endStroke()
{
    std::optional<OpenGLEngine::StrokeResult> stroke = m_gpu.endStroke();
    if (!stroke || !m_doc.find(stroke->layer))
        return;
    doc::PixelEditRecorder rec(m_doc, stroke->layer);
    rec.touch(stroke->rect);
    core::Image<std::uint8_t> &pixels = rec.image();
    const core::Recti r = stroke->rect;
    for (int y = 0; y < r.h; ++y)
        std::copy_n(stroke->pixels.row(y), std::size_t(r.w) * 4, pixels.row(r.y + y) + std::size_t(r.x) * 4);
    rec.commit(m_stroke_label);
}

core::Image<std::uint8_t> App::activePixels() const
{
    const doc::Layer *layer = m_doc.activeLayer();
    return layer ? core::toRGB8(layer->pixels()) : core::Image<std::uint8_t>();
}

std::optional<std::array<std::uint8_t, 4>> App::pickColor(core::Vec2i p, bool composite)
{
    if (m_doc.empty() || p.x < 0 || p.y < 0 || p.x >= m_doc.width() || p.y >= m_doc.height())
        return std::nullopt;
    if (composite)
    {
        m_gpu.sync(m_doc);
        const core::Image<std::uint8_t> px = m_gpu.compositeTexture().download8({p.x, p.y, 1, 1});
        return std::array<std::uint8_t, 4>{px(0, 0, 0), px(0, 0, 1), px(0, 0, 2), px(0, 0, 3)};
    }
    const doc::Layer *layer = m_doc.activeLayer();
    if (!layer)
        return std::nullopt;
    const core::Image<std::uint8_t> &px = layer->pixels();
    return std::array<std::uint8_t, 4>{px(p.x, p.y, 0), px(p.x, p.y, 1), px(p.x, p.y, 2), px(p.x, p.y, 3)};
}

bool App::bucketFill(core::Vec2i p, float threshold, int compare, std::array<std::uint8_t, 4> rgba)
{
    const doc::Layer *layer = m_doc.activeLayer();
    if (!layer || p.x < 0 || p.y < 0 || p.x >= m_doc.width() || p.y >= m_doc.height())
        return false;
    if (layer->props().locked)
    {
        m_log.warn("{} is locked; nothing filled", layer->props().name);
        return false;
    }
    endStroke();
    doc::Shape region;
    // Limited to the selection: the region can't leak out through an unselected gap
    const double ms = millisecondsOf([&] { region = growRegion(activePixels(), p, threshold, compare, m_doc.hasSelection() ? &m_doc.selection() : nullptr); });
    const bool filled = doc::fillMask(m_doc, layer->id(), region.mask, region.bounds, rgba, "Bucket fill");
    m_log.info("Bucket fill at {}, {}: {} x {} box, {:.0f} ms", p.x, p.y, region.bounds.w, region.bounds.h, ms);
    return filled;
}

bool App::magicWand(core::Vec2i p, float threshold, int compare, doc::SelectMode mode)
{
    if (!m_doc.activeLayer() || p.x < 0 || p.y < 0 || p.x >= m_doc.width() || p.y >= m_doc.height())
        return false;
    endStroke();
    doc::Shape region;
    const double ms = millisecondsOf([&] { region = growRegion(activePixels(), p, threshold, compare); });
    const std::string label = mode == doc::SelectMode::Replace ? "Magic wand" : std::string("Magic wand (") + doc::selectModeName(mode) + ")";
    m_log.info("Magic wand at {}, {}: {} x {} box, {:.0f} ms", p.x, p.y, region.bounds.w, region.bounds.h, ms);
    return doc::applySelection(m_doc, region, mode, label);
}

core::Image<std::uint8_t> App::runOnActive(const std::function<Mat<uint8_t>(const Mat<uint8_t> &, const Mat<uint8_t> *)> &run, double &ms)
{
    const core::Image<std::uint8_t> source = activePixels();
    const Mat<uint8_t> input = toMat(source);
    Mat<uint8_t> roi;
    const bool limited = m_doc.hasSelection();
    if (limited)
        roi = toMat(m_doc.selection());
    Mat<uint8_t> output;
    ms = millisecondsOf([&] { output = run(input, limited ? &roi : nullptr); });
    core::Image<std::uint8_t> result = copyOf(viewOf(output));
    // Outside the selection the result shows the source, not black
    if (limited && result.channels() == source.channels())
        for (int y = 0; y < result.height(); ++y)
            for (int x = 0; x < result.width(); ++x)
                if (m_doc.selection()(x, y) < 128)
                    for (int c = 0; c < result.channels(); ++c)
                        result(x, y, c) = source(x, y, c);
    return result;
}

void App::runSlic(const SlicParameter &param, int coloration)
{
    const doc::Layer *layer = m_doc.activeLayer();
    if (!layer)
        return;
    endStroke();
    double ms = 0;
    core::Image<std::uint8_t> result = runOnActive([&](const Mat<uint8_t> &in, const Mat<uint8_t> *roi) { return m_rg.dispatch(param, in, coloration, roi); }, ms);
    const char *where = m_doc.hasSelection() ? " in the selection" : "";
    m_result_source = layer->id();
    setResult(std::move(result), std::format("SLIC K {}", param.m_K), std::format("SLIC: K {}, compactness {}{}", param.m_K, param.m_m, where));
    m_log.info("SLIC on {}{}: K {}, compactness {}, {:.0f} ms", layer->props().name, where, param.m_K, param.m_m, ms);
}

void App::runSeeded(const SeedRGParameter &settings, int coloration)
{
    const doc::Layer *layer = m_doc.activeLayer();
    if (!layer)
        return;
    SeedRGParameter param = settings;
    if (!param.m_random)
    {
        if (m_doc.seeds().empty())
        {
            m_log.warn("No seeds placed; use the Seeds tool first");
            return;
        }
        param.m_points.clear();
        for (core::Vec2i p : m_doc.seeds())
            param.m_points.push_back({p.x, p.y});
        param.m_seednb = int(param.m_points.size());
    }
    endStroke();
    double ms = 0;
    core::Image<std::uint8_t> result = runOnActive([&](const Mat<uint8_t> &in, const Mat<uint8_t> *roi) { return m_rg.dispatch(param, in, coloration, roi); }, ms);
    const char *where = m_doc.hasSelection() ? " in the selection" : "";
    m_result_source = layer->id();
    setResult(std::move(result), std::format("Seeded {} seeds", param.m_seednb),
              std::format("Seeded growing: {} seeds, threshold {:.1f}{}", param.m_seednb, param.m_threshold, where));
    m_log.info("Seeded growing on {}{}: {} seeds, threshold {:.1f}, {:.0f} ms", layer->props().name, where, param.m_seednb, param.m_threshold, ms);
}

void App::setResult(core::Image<std::uint8_t> image, std::string label, std::string origin)
{
    m_result = std::move(image);
    m_result_label = std::move(label);
    m_result_origin = std::move(origin);
    m_gpu.setResult(m_result);
}

bool App::saveResult(const std::string &path)
{
    std::string error;
    if (!core::saveImage(path, m_result, &error))
    {
        m_log.error("{}", error);
        return false;
    }
    m_log.info("Saved result to {}", path);
    return true;
}

void App::discardResult()
{
    setResult(core::Image<std::uint8_t>(), {}, {});
    m_result_source = 0;
}

void App::addResultAsLayer()
{
    if (!hasResult() || m_doc.empty())
        return;
    if (m_result.width() != m_doc.width() || m_result.height() != m_doc.height())
    {
        m_log.error("The result and the image differ in size; no layer added");
        return;
    }
    // Above its source when that still exists, else above the active layer
    const int source = m_doc.indexOf(m_result_source);
    doc::addLayer(m_doc, doc::Layer(m_result_label, core::toRGBA8(m_result), doc::LayerKind::Result, m_result_origin),
                  source >= 0 ? source + 1 : -1, "Add " + m_result_label);
}

void App::copyResultToLayer()
{
    const doc::Layer *layer = m_doc.activeLayer();
    if (!hasResult() || !layer)
        return;
    if (m_result.width() != m_doc.width() || m_result.height() != m_doc.height())
    {
        m_log.error("The result and the image differ in size; nothing copied");
        return;
    }
    if (layer->props().locked)
    {
        m_log.warn("{} is locked; nothing copied", layer->props().name);
        return;
    }
    endStroke();
    doc::replacePixels(m_doc, layer->id(), m_result, "Copy " + m_result_label);
}

void App::updateTitle()
{
    m_platform.setTitle(m_image_path.empty() ? "SeeThing" : fileName(m_save_path.empty() ? m_image_path : m_save_path) + " - SeeThing");
}

#include "App.h"

#include "AssetPath.h"
#include "MatBridge.h"
#include "core/ImageIO.h"

#include <chrono>
#include <filesystem>

namespace
{
    std::string fileName(const std::string &path)
    {
        return std::filesystem::path(path).filename().string();
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
    m_gpu.setMainImage(loaded.image);
    m_gpu.clearResult();
    // Only layer 0 for now: replace it, don't stack a new one
    Layer layer(std::move(loaded.image));
    if (m_canvas.getLayerNb() > 0)
        m_canvas.getLayer(0) = std::move(layer);
    else
        m_canvas.addLayer(std::move(layer));

    m_image_path = path;
    m_save_path.clear();
    const Layer &l = m_canvas.getLayer(0);
    m_log.info("Opened {} ({} x {}, {} channels)", path, l.getWidth(), l.getHeight(), l.image.channels());
    updateTitle();
    return true;
}

bool App::saveImage(const std::string &path)
{
    std::string error;
    if (!m_gpu.saveMain(path, &error))
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
    m_gpu.clearMain();
    m_gpu.clearResult();
    m_canvas.clear();
    m_image_path.clear();
    m_save_path.clear();
    updateTitle();
}

void App::runSlic(const SlicParameter &param, int coloration)
{
    if (m_canvas.getLayerNb() == 0)
        return;
    Mat<uint8_t> input = toMat(m_canvas.getLayer(0).getImage());
    Mat<uint8_t> output;
    const double ms = millisecondsOf([&] { output = m_rg.dispatch(param, input, coloration); });
    m_gpu.setResult(viewOf(output));
    m_log.info("SLIC: K {}, compactness {}, {:.0f} ms", param.m_K, param.m_m, ms);
}

void App::runSeeded(const SeedRGParameter &param, int coloration)
{
    if (m_canvas.getLayerNb() == 0)
        return;
    Mat<uint8_t> input = toMat(m_canvas.getLayer(0).getImage());
    Mat<uint8_t> output;
    const double ms = millisecondsOf([&] { output = m_rg.dispatch(param, input, coloration); });
    m_gpu.setResult(viewOf(output));
    m_log.info("Seeded growing: {} seeds, threshold {:.1f}, {:.0f} ms", param.m_seednb, param.m_threshold, ms);
}

bool App::saveResult(const std::string &path)
{
    std::string error;
    if (!m_gpu.saveResult(path, &error))
    {
        m_log.error("{}", error);
        return false;
    }
    m_log.info("Saved result to {}", path);
    return true;
}

void App::updateTitle()
{
    m_platform.setTitle(m_image_path.empty() ? "SeeThing" : fileName(m_save_path.empty() ? m_image_path : m_save_path) + " - SeeThing");
}

#ifndef EDITOR_H
#define EDITOR_H
#include "StructParam.h"

// Engines
#include "RegionGrowingEngine.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <span>
#include <string>

#include "gpu/Framebuffer.h"
#include "gpu/Renderer.h"
#include "gpu/Texture.h"

#include "Canvas.h"

class OpenGLEngine;

namespace appobj
{
    // Created by ImgApp once the GL context exists, destroyed before it goes
    extern std::unique_ptr<OpenGLEngine> glengine;
    extern Canvas canvas;
}

class OpenGLEngine
{
public:
    explicit OpenGLEngine(const std::string &shader_dir);

    // Loads into layer 0 and the main texture
    bool openImage(const std::string &path, std::string *error = nullptr);
    bool hasImage() const { return !m_main_tex.empty(); }
    const gpu::Texture &mainTexture() const { return m_main_tex; }
    bool saveMain(const std::string &path, std::string *error = nullptr) const;

    // Pixel coordinates in the main image
    void paint(std::span<const core::Vec2f> pixels, core::Vec3f color, float size = 2.0f);

    void setResult(core::ImageView<const uint8_t> image);
    bool hasResult() const { return !m_result_tex.empty(); }
    const gpu::Texture &resultTexture() const { return m_result_tex; }
    void clearResult();
    bool saveResult(const std::string &path, std::string *error = nullptr) const;
    // Copies the result over the main texture (not over layer 0 yet); false when the sizes differ
    bool transferResultToMain();

    // Call once per frame, after ImGui rendered
    void endFrame();

private:
    gpu::Renderer m_renderer;
    gpu::Texture m_main_tex;
    gpu::Framebuffer m_main_fbo;
    gpu::Texture m_result_tex;
    std::chrono::steady_clock::time_point m_last_shader_check;
};

#endif

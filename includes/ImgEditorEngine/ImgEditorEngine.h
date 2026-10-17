#ifndef EDITOR_H
#define EDITOR_H
#include <chrono>
#include <cstdint>
#include <span>
#include <string>

#include "core/Image.h"
#include "gpu/Framebuffer.h"
#include "gpu/Renderer.h"
#include "gpu/Texture.h"

namespace core
{
    class Log;
}

class OpenGLEngine
{
public:
    // Needs a current GL context; must be destroyed before it
    OpenGLEngine(const std::string &shader_dir, core::Log &log);

    // Shown as RGBA8 whatever the image holds
    void setMainImage(core::ImageView<const uint8_t> image);
    void clearMain();
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

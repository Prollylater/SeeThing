#include "ImgEditorEngine.h"

#include "core/Color.h"
#include "core/ImageIO.h"

namespace appobj
{
    Canvas canvas;
    std::unique_ptr<OpenGLEngine> glengine;
    RGEngine rgengine;
}

OpenGLEngine::OpenGLEngine(const std::string &shader_dir) : m_renderer(shader_dir)
{
}

bool OpenGLEngine::openImage(const std::string &path, std::string *error)
{
    core::LoadResult loaded = core::loadImage(path, 0);
    if (!loaded.ok())
    {
        if (error)
            *error = loaded.error;
        return false;
    }

    // RGBA8 whatever the file holds: the main texture is a render target
    gpu::Texture tex = gpu::Texture::fromImage<uint8_t>(core::toRGBA8(loaded.image));
    gpu::Framebuffer fbo(tex);
    // The old texture may still be drawn this frame
    m_renderer.retire(std::move(m_main_fbo));
    m_renderer.retire(std::move(m_main_tex));
    m_main_tex = std::move(tex);
    m_main_fbo = std::move(fbo);

    // Currently only deal with the layer 0: replace it, don't stack a new one
    Layer layer(std::move(loaded.image));
    if (appobj::canvas.getLayerNb() > 0)
        appobj::canvas.getLayer(0) = std::move(layer);
    else
        appobj::canvas.addLayer(std::move(layer));
    return true;
}

bool OpenGLEngine::saveMain(const std::string &path, std::string *error) const
{
    if (!hasImage())
    {
        if (error)
            *error = "No image to save";
        return false;
    }
    return core::saveImage(path, m_main_tex.download8(), error);
}

void OpenGLEngine::paint(std::span<const core::Vec2f> pixels, core::Vec3f color, float size)
{
    if (hasImage())
        m_renderer.drawPoints(m_main_fbo, pixels, color, size);
}

void OpenGLEngine::setResult(core::ImageView<const uint8_t> image)
{
    m_renderer.retire(std::move(m_result_tex));
    if (!image.empty())
        m_result_tex = gpu::Texture::fromImage(image);
}

void OpenGLEngine::clearResult()
{
    m_renderer.retire(std::move(m_result_tex));
}

bool OpenGLEngine::saveResult(const std::string &path, std::string *error) const
{
    if (!hasResult())
    {
        if (error)
            *error = "No result to save";
        return false;
    }
    return core::saveImage(path, m_result_tex.download8(), error);
}

bool OpenGLEngine::transferResultToMain()
{
    if (!hasImage() || !hasResult() || m_result_tex.width() != m_main_tex.width() || m_result_tex.height() != m_main_tex.height())
        return false;
    m_renderer.blit(m_result_tex, m_main_fbo);
    return true;
}

void OpenGLEngine::endFrame()
{
    m_renderer.endFrame();
#ifndef NDEBUG
    // Edited shaders apply without a restart
    const auto now = std::chrono::steady_clock::now();
    if (now - m_last_shader_check > std::chrono::milliseconds(500))
    {
        m_last_shader_check = now;
        m_renderer.shaders().reloadChanged();
    }
#endif
}

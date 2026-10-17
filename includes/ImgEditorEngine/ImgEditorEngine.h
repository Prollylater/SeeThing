#ifndef EDITOR_H
#define EDITOR_H
#include <chrono>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>

#include "core/Image.h"
#include "doc/Document.h"
#include "gpu/Framebuffer.h"
#include "gpu/Renderer.h"
#include "gpu/Texture.h"

namespace core
{
    class Log;
}

// The GPU mirror of the document: one texture per layer, the selection mask, and their composite.
// The document stays the truth; strokes paint here first and are read back when they end.
class OpenGLEngine
{
public:
    // Needs a current GL context; must be destroyed before it
    OpenGLEngine(const std::string &shader_dir, core::Log &log);

    void sync(doc::Document &doc);
    void clear();
    // RGBA8, straight alpha, the document size
    const gpu::Texture &compositeTexture() const { return m_composite; }
    const gpu::Texture &selectionTexture() const { return m_selection; }
    bool saveComposite(const std::string &path, std::string *error = nullptr) const;

    struct StrokeResult
    {
        doc::LayerId layer = 0;
        core::Recti rect;
        core::Image<std::uint8_t> pixels;
    };
    // Paints straight into the layer's texture until endStroke
    void beginStroke(doc::Document &doc, doc::LayerId layer);
    // Pixel coordinates; clips to the selection when one exists
    void strokePoints(const doc::Document &doc, std::span<const core::Vec2f> pixels, core::Vec3f color, float size);
    std::optional<StrokeResult> endStroke();
    bool stroking() const { return m_stroke.has_value(); }

    void setResult(core::ImageView<const uint8_t> image);
    bool hasResult() const { return !m_result_tex.empty(); }
    const gpu::Texture &resultTexture() const { return m_result_tex; }
    void clearResult();

    gpu::Renderer &renderer() { return m_renderer; }
    // Call once per frame, after ImGui rendered
    void endFrame();

private:
    struct LayerGpu
    {
        gpu::Texture tex;
        gpu::Framebuffer fbo;
    };

    void resize(int width, int height);
    void composite(const doc::Document &doc);

    gpu::Renderer m_renderer;
    std::uint64_t m_serial = 0, m_revision = 0;
    std::unordered_map<doc::LayerId, LayerGpu> m_layers;
    gpu::Texture m_selection;
    gpu::Texture m_composite;
    gpu::Framebuffer m_composite_fbo;
    
    gpu::Texture m_scratch[2];
    gpu::Framebuffer m_scratch_fbo[2];
    bool m_stale = true;

    struct Stroke
    {
        doc::LayerId layer;
        core::Recti rect;
    };
    std::optional<Stroke> m_stroke;

    gpu::Texture m_result_tex;
    std::chrono::steady_clock::time_point m_last_shader_check;
};

#endif

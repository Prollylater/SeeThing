#include "ImgEditorEngine.h"

#include "core/ImageIO.h"

#include <cmath>
#include <vector>

OpenGLEngine::OpenGLEngine(const std::string &shader_dir, core::Log &log) : m_renderer(shader_dir, &log)
{
}

void OpenGLEngine::clear()
{
    // Textures may still be drawn this frame
    for (auto &[id, l] : m_layers)
    {
        m_renderer.retire(std::move(l.fbo));
        m_renderer.retire(std::move(l.tex));
    }
    m_layers.clear();
    m_renderer.retire(std::move(m_selection));
    m_renderer.retire(std::move(m_composite_fbo));
    m_renderer.retire(std::move(m_composite));
    for (int i = 0; i < 2; ++i)
    {
        m_renderer.retire(std::move(m_scratch_fbo[i]));
        m_renderer.retire(std::move(m_scratch[i]));
    }
    m_renderer.retire(std::move(m_coverage_fbo));
    m_renderer.retire(std::move(m_coverage));
    m_renderer.retire(std::move(m_applied_fbo));
    m_renderer.retire(std::move(m_applied));
    m_stroke.reset();
    m_serial = 0;
    m_stale = true;
}

void OpenGLEngine::resize(int width, int height)
{
    clear();
    m_selection = gpu::Texture(width, height, core::PixelFormat::R8);
    m_composite = gpu::Texture(width, height, core::PixelFormat::RGBA8);
    m_composite_fbo = gpu::Framebuffer(m_composite);
    for (int i = 0; i < 2; ++i)
    {
        m_scratch[i] = gpu::Texture(width, height, core::PixelFormat::RGBA8);
        m_scratch_fbo[i] = gpu::Framebuffer(m_scratch[i]);
    }
    // Float: low-flow dabs add tiny amounts that 8 bits would round away
    m_coverage = gpu::Texture(width, height, core::PixelFormat::R32F);
    m_coverage_fbo = gpu::Framebuffer(m_coverage);
    m_applied = gpu::Texture(width, height, core::PixelFormat::RGBA8);
    m_applied_fbo = gpu::Framebuffer(m_applied);
}

void OpenGLEngine::sync(doc::Document &doc)
{
    if (doc.empty())
    {
        if (m_serial != 0)
            clear();
        return;
    }
    if (doc.serial() != m_serial)
    {
        resize(doc.width(), doc.height());
        m_serial = doc.serial();
        m_selection.upload(doc.selection());
        doc.takeSelectionDirty();
    }
    else if (doc.revision() == m_revision && !m_stale)
    {
        return;
    }

    // Layers gone from the document lose their textures; an undo brings them back with a full upload
    for (auto it = m_layers.begin(); it != m_layers.end();)
    {
        if (doc.find(it->first) || (m_stroke && m_stroke->layer == it->first))
        {
            ++it;
            continue;
        }
        m_renderer.retire(std::move(it->second.fbo));
        m_renderer.retire(std::move(it->second.tex));
        it = m_layers.erase(it);
    }
    for (int i = 0; i < doc.layerCount(); ++i)
    {
        const doc::Layer &layer = doc.layer(i);
        const core::Recti dirty = doc.takeDirty(layer.id());
        auto it = m_layers.find(layer.id());
        if (it == m_layers.end())
        {
            LayerGpu l;
            l.tex = gpu::Texture::fromImage<std::uint8_t>(layer.pixels());
            l.fbo = gpu::Framebuffer(l.tex);
            m_layers.emplace(layer.id(), std::move(l));
        }
        else if (!dirty.empty())
        {
            it->second.tex.upload(dirty.x, dirty.y, layer.pixels().view().sub(dirty.x, dirty.y, dirty.w, dirty.h));
        }
    }
    if (const core::Recti dirty = doc.takeSelectionDirty(); !dirty.empty())
        m_selection.upload(dirty.x, dirty.y, doc.selection().view().sub(dirty.x, dirty.y, dirty.w, dirty.h));

    composite(doc);
    m_revision = doc.revision();
    m_stale = false;
}

// Bottom to top, each visible layer over what is below; the last pass writes the composite
void OpenGLEngine::composite(const doc::Document &doc)
{
    std::vector<const doc::Layer *> visible;
    for (int i = 0; i < doc.layerCount(); ++i)
        if (doc.layer(i).props().visible && doc.layer(i).props().opacity > 0.0f)
            visible.push_back(&doc.layer(i));
    if (visible.empty())
    {
        m_renderer.clear(m_composite_fbo);
        return;
    }
    m_renderer.clear(m_scratch_fbo[0]);
    int backdrop = 0;
    for (std::size_t i = 0; i < visible.size(); ++i)
    {
        const doc::LayerProps &p = visible[i]->props();
        const bool last = i + 1 == visible.size();
        const gpu::Framebuffer &dst = last ? m_composite_fbo : m_scratch_fbo[1 - backdrop];
        const gpu::Texture *source = &m_layers.at(visible[i]->id()).tex;
        // A brush stroke in progress shows on its layer before it is committed
        if (m_stroke && m_stroke->stamp && m_stroke->layer == visible[i]->id())
        {
            applyStamp(*m_stroke, *source);
            source = &m_applied;
        }
        m_renderer.composite(m_scratch[backdrop], *source, p.opacity, p.blend, dst);
        backdrop = 1 - backdrop;
    }
}

bool OpenGLEngine::saveComposite(const std::string &path, std::string *error) const
{
    if (m_composite.empty())
    {
        if (error)
            *error = "No image to save";
        return false;
    }
    return core::saveImage(path, m_composite.download8(), error);
}

void OpenGLEngine::beginStroke(doc::Document &doc, doc::LayerId layer)
{
    sync(doc);
    m_stroke = Stroke{layer, {}, std::nullopt, doc.hasSelection()};
}

void OpenGLEngine::beginStamp(doc::Document &doc, doc::LayerId layer, StampStyle style)
{
    sync(doc);
    if (m_coverage.empty())
        return;
    m_renderer.clear(m_coverage_fbo);
    m_stroke = Stroke{layer, {}, style, doc.hasSelection()};
}

void OpenGLEngine::stampDabs(std::span<const core::Vec2f> centers, float diameter, float hardness, float flow)
{
    if (!m_stroke || !m_stroke->stamp || centers.empty())
        return;
    m_renderer.drawDabs(m_coverage_fbo, centers, diameter, hardness, flow);
    const int reach = int(std::ceil(diameter * 0.5f)) + 2;
    for (core::Vec2f c : centers)
        m_stroke->rect = core::unite(m_stroke->rect, {int(std::floor(c.x)) - reach, int(std::floor(c.y)) - reach, 2 * reach + 1, 2 * reach + 1});
    m_stroke->rect = core::intersect(m_stroke->rect, {0, 0, m_coverage.width(), m_coverage.height()});
    m_stale = true;
}

void OpenGLEngine::applyStamp(const Stroke &stroke, const gpu::Texture &layer)
{
    const StampStyle &st = *stroke.stamp;
    m_renderer.applyStroke(layer, m_coverage, stroke.clip ? &m_selection : nullptr, st.color, st.opacity, st.erase, m_applied_fbo);
}

void OpenGLEngine::strokePoints(const doc::Document &doc, std::span<const core::Vec2f> pixels, core::Vec3f color, float size)
{
    if (!m_stroke || pixels.empty())
        return;
    auto it = m_layers.find(m_stroke->layer);
    if (it == m_layers.end())
        return;
    if (m_stroke->stamp)
        return;
    m_renderer.drawPoints(it->second.fbo, pixels, color, size, doc.hasSelection() ? &m_selection : nullptr);
    // Each square spans pixel + 0.5 +- size / 2
    const int reach = int(std::ceil(size * 0.5f));
    for (core::Vec2f p : pixels)
    {
        const int x = int(p.x), y = int(p.y);
        m_stroke->rect = core::unite(m_stroke->rect, {x - reach, y - reach, 2 * reach + 1, 2 * reach + 1});
    }
    m_stroke->rect = core::intersect(m_stroke->rect, {0, 0, it->second.tex.width(), it->second.tex.height()});
    m_stale = true;
}

std::optional<OpenGLEngine::StrokeResult> OpenGLEngine::endStroke()
{
    if (!m_stroke)
        return std::nullopt;
    const Stroke stroke = *m_stroke;
    m_stroke.reset();
    m_stale = true;
    auto it = m_layers.find(stroke.layer);
    if (it == m_layers.end() || stroke.rect.empty())
        return std::nullopt;
    if (stroke.stamp)
    {
        applyStamp(stroke, it->second.tex);
        return StrokeResult{stroke.layer, stroke.rect, m_applied.download8(stroke.rect)};
    }
    return StrokeResult{stroke.layer, stroke.rect, it->second.tex.download8(stroke.rect)};
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

void OpenGLEngine::endFrame()
{
    m_renderer.endFrame();
#ifndef NDEBUG
    // Edited shaders apply without a restart
    const auto now = std::chrono::steady_clock::now();
    if (now - m_last_shader_check > std::chrono::milliseconds(500))
    {
        m_last_shader_check = now;
        if (m_renderer.shaders().reloadChanged() > 0)
            m_stale = true;
    }
#endif
}

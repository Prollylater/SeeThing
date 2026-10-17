#ifndef GPU_RENDERER_H
#define GPU_RENDERER_H

#include "Framebuffer.h"
#include "ShaderCache.h"
#include "Texture.h"
#include "core/BlendMode.h"
#include "core/Geometry.h"

#include <span>
#include <string>
#include <vector>

namespace gpu
{
    // Where the selection overlay lands, in ImGui coordinates of the viewport being drawn
    struct SelectionOverlay
    {
        core::Vec2f viewportPos, viewportSize;
        core::Vec2f imageOrigin;
        float zoom = 1.0f;
        float time = 0.0f;
    };

    // Image passes and deferred deletes. Passes leave framebuffer, program and VAO unbound.
    class Renderer
    {
    public:
        explicit Renderer(const std::string &shaderDir, core::Log *log = nullptr);

        void clear(const Framebuffer &dst, float r = 0, float g = 0, float b = 0, float a = 0);
        // Draws src over the whole target, nearest-sampled
        void blit(const Texture &src, const Framebuffer &dst);
        // dst = source over backdrop with that mode and opacity; all three the same size, dst neither input
        void composite(const Texture &backdrop, const Texture &source, float opacity, core::BlendMode mode, const Framebuffer &dst);
        // Pixel coordinates, (0, 0) the top-left pixel; each point is a size x size square.
        // With a clip mask (R8, the target's size) only pixels where it is at least one half change.
        void drawPoints(const Framebuffer &dst, std::span<const core::Vec2f> pixels, core::Vec3f color, float size,
                        const Texture *clip = nullptr);
        // For an ImGui draw callback: draws into ImGui's framebuffer with its blending and scissor, and its VAO,
        // since VAOs are not shared with the other viewports' contexts. Reset ImGui's render state afterwards.
        void drawSelectionOverlay(const Texture &mask, const SelectionOverlay &where);

        // ImGui may still draw a texture released this frame: keep it until endFrame(), after ImGui rendered
        void retire(Texture &&tex);
        void retire(Framebuffer &&fbo);
        void endFrame();
        std::size_t retiredCount() const { return m_retiredTextures.size() + m_retiredFramebuffers.size(); }

        ShaderCache &shaders() { return m_shaders; }

    private:
        void beginPass(const Framebuffer &dst);
        void endPass();

        ShaderCache m_shaders;
        VertexArrayHandle m_emptyVao;
        VertexArrayHandle m_pointsVao;
        BufferHandle m_pointsVbo;
        SamplerHandle m_nearest;
        std::vector<Texture> m_retiredTextures;
        std::vector<Framebuffer> m_retiredFramebuffers;
    };
}

#endif

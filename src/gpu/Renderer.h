#ifndef GPU_RENDERER_H
#define GPU_RENDERER_H

#include "Framebuffer.h"
#include "ShaderCache.h"
#include "Texture.h"
#include "core/Geometry.h"

#include <span>
#include <string>
#include <vector>

namespace gpu
{
    // Image passes and deferred deletes. Passes leave framebuffer, program and VAO unbound.
    class Renderer
    {
    public:
        // Loads fullscreen.vert, blit.frag, points.vert and solid.frag from shaderDir; throws if one fails
        explicit Renderer(const std::string &shaderDir);

        // Draws src over the whole target, nearest-sampled
        void blit(const Texture &src, const Framebuffer &dst);
        // Pixel coordinates, (0, 0) the top-left pixel; each point is a size x size square
        void drawPoints(const Framebuffer &dst, std::span<const core::Vec2f> pixels, core::Vec3f color, float size);

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

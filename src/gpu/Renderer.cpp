#include "Renderer.h"

#include <stdexcept>

namespace gpu
{
    Renderer::Renderer(const std::string &shaderDir, core::Log *log) : m_shaders(log)
    {
        const std::string dir = shaderDir.empty() || shaderDir.back() == '/' ? shaderDir : shaderDir + "/";
        if (!m_shaders.load("blit", dir + "fullscreen.vert", dir + "blit.frag") ||
            !m_shaders.load("points", dir + "points.vert", dir + "solid.frag"))
            throw std::runtime_error("gpu::Renderer: " + m_shaders.lastError());

        // The fullscreen triangle comes from gl_VertexID; core profile still wants a VAO bound
        m_emptyVao = VertexArrayHandle::create();

        m_pointsVbo = BufferHandle::create();
        m_pointsVao = VertexArrayHandle::create();
        glVertexArrayVertexBuffer(m_pointsVao.get(), 0, m_pointsVbo.get(), 0, sizeof(core::Vec2f));
        glVertexArrayAttribFormat(m_pointsVao.get(), 0, 2, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(m_pointsVao.get(), 0, 0);
        glEnableVertexArrayAttrib(m_pointsVao.get(), 0);
        glVertexArrayBindingDivisor(m_pointsVao.get(), 0, 1);

        m_nearest = SamplerHandle::create();
        glSamplerParameteri(m_nearest.get(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glSamplerParameteri(m_nearest.get(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glSamplerParameteri(m_nearest.get(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(m_nearest.get(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    void Renderer::beginPass(const Framebuffer &dst)
    {
        if (dst.empty())
            throw std::logic_error("gpu::Renderer: empty target");
        dst.bind();
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
    }

    // ImGui restores the program bound before it drew; NVIDIA then recompiles it for ImGui's state
    void Renderer::endPass()
    {
        glBindVertexArray(0);
        glUseProgram(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Renderer::blit(const Texture &src, const Framebuffer &dst)
    {
        beginPass(dst);
        const Program &prog = m_shaders.get("blit");
        prog.use();
        glBindTextureUnit(0, src.id());
        glBindSampler(0, m_nearest.get());
        glBindVertexArray(m_emptyVao.get());
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindSampler(0, 0);
        endPass();
    }

    void Renderer::drawPoints(const Framebuffer &dst, std::span<const core::Vec2f> pixels, core::Vec3f color, float size)
    {
        if (pixels.empty())
            return;
        beginPass(dst);
        // Orphan and refill: the driver keeps the old storage until the previous draw is done
        glNamedBufferData(m_pointsVbo.get(), GLsizeiptr(pixels.size_bytes()), pixels.data(), GL_STREAM_DRAW);
        const Program &prog = m_shaders.get("points");
        prog.set("target_size", core::Vec2f{float(dst.width()), float(dst.height())});
        prog.set("point_size", size);
        prog.set("color", color);
        prog.use();
        // One instanced square per point: GL_POINTS caps the size per driver
        glBindVertexArray(m_pointsVao.get());
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, GLsizei(pixels.size()));
        endPass();
    }

    void Renderer::retire(Texture &&tex)
    {
        if (!tex.empty())
            m_retiredTextures.push_back(std::move(tex));
    }

    void Renderer::retire(Framebuffer &&fbo)
    {
        if (!fbo.empty())
            m_retiredFramebuffers.push_back(std::move(fbo));
    }

    void Renderer::endFrame()
    {
        m_retiredFramebuffers.clear();
        m_retiredTextures.clear();
    }
}

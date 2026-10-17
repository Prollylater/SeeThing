#ifndef GPU_FRAMEBUFFER_H
#define GPU_FRAMEBUFFER_H

#include "GlHandle.h"
#include "Texture.h"

namespace gpu
{
    // Renders into one texture; the texture must outlive it
    class Framebuffer
    {
    public:
        Framebuffer() = default;
        // Throws when the attachment is not renderable
        explicit Framebuffer(Texture &color);

        // Binds it and sets the viewport to the texture size
        void bind() const;

        GLuint id() const { return m_fbo.get(); }
        int width() const { return m_w; }
        int height() const { return m_h; }
        bool empty() const { return !m_fbo; }

    private:
        FramebufferHandle m_fbo;
        int m_w = 0, m_h = 0;
    };
}

#endif

#include "Framebuffer.h"

#include <stdexcept>
#include <string>

namespace gpu
{
    namespace
    {
        const char *statusName(GLenum status)
        {
            switch (status)
            {
            case GL_FRAMEBUFFER_UNDEFINED: return "undefined";
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: return "incomplete attachment";
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "missing attachment";
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: return "incomplete draw buffer";
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: return "incomplete read buffer";
            case GL_FRAMEBUFFER_UNSUPPORTED: return "unsupported format";
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: return "incomplete multisample";
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: return "incomplete layer targets";
            }
            return "unknown status";
        }
    }

    Framebuffer::Framebuffer(Texture &color) : m_w(color.width()), m_h(color.height())
    {
        if (color.empty())
            throw std::invalid_argument("gpu::Framebuffer: empty texture");
        m_fbo = FramebufferHandle::create();
        glNamedFramebufferTexture(m_fbo.get(), GL_COLOR_ATTACHMENT0, color.id(), 0);
        const GLenum status = glCheckNamedFramebufferStatus(m_fbo.get(), GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error(std::string("gpu::Framebuffer: ") + statusName(status));
    }

    void Framebuffer::bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo.get());
        glViewport(0, 0, m_w, m_h);
    }
}

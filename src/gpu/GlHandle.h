#ifndef GPU_GLHANDLE_H
#define GPU_GLHANDLE_H

#include <glad/gl.h>

#include <utility>

// Move-only owners of GL objects. They must die while the context is alive: no GL objects in globals or statics.
namespace gpu
{
    template <class Traits>
    class GlHandle
    {
    public:
        GlHandle() = default;
        explicit GlHandle(GLuint id) : m_id(id) {}
        static GlHandle create() { return GlHandle(Traits::create()); }

        ~GlHandle() { reset(); }
        GlHandle(GlHandle &&o) noexcept : m_id(std::exchange(o.m_id, 0)) {}
        GlHandle &operator=(GlHandle &&o) noexcept
        {
            if (this != &o)
            {
                reset();
                m_id = std::exchange(o.m_id, 0);
            }
            return *this;
        }
        GlHandle(const GlHandle &) = delete;
        GlHandle &operator=(const GlHandle &) = delete;

        GLuint get() const { return m_id; }
        explicit operator bool() const { return m_id != 0; }
        void reset()
        {
            if (m_id)
                Traits::destroy(m_id);
            m_id = 0;
        }

    private:
        GLuint m_id = 0;
    };

    struct TextureTraits
    {
        static GLuint create() { GLuint id = 0; glCreateTextures(GL_TEXTURE_2D, 1, &id); return id; }
        static void destroy(GLuint id) { glDeleteTextures(1, &id); }
    };
    struct BufferTraits
    {
        static GLuint create() { GLuint id = 0; glCreateBuffers(1, &id); return id; }
        static void destroy(GLuint id) { glDeleteBuffers(1, &id); }
    };
    struct VertexArrayTraits
    {
        static GLuint create() { GLuint id = 0; glCreateVertexArrays(1, &id); return id; }
        static void destroy(GLuint id) { glDeleteVertexArrays(1, &id); }
    };
    struct FramebufferTraits
    {
        static GLuint create() { GLuint id = 0; glCreateFramebuffers(1, &id); return id; }
        static void destroy(GLuint id) { glDeleteFramebuffers(1, &id); }
    };
    struct SamplerTraits
    {
        static GLuint create() { GLuint id = 0; glCreateSamplers(1, &id); return id; }
        static void destroy(GLuint id) { glDeleteSamplers(1, &id); }
    };
    struct ProgramTraits
    {
        static GLuint create() { return glCreateProgram(); }
        static void destroy(GLuint id) { glDeleteProgram(id); }
    };
    // Built from glCreateShader(stage), which needs the stage
    struct ShaderTraits
    {
        static void destroy(GLuint id) { glDeleteShader(id); }
    };

    using TextureHandle = GlHandle<TextureTraits>;
    using BufferHandle = GlHandle<BufferTraits>;
    using VertexArrayHandle = GlHandle<VertexArrayTraits>;
    using FramebufferHandle = GlHandle<FramebufferTraits>;
    using SamplerHandle = GlHandle<SamplerTraits>;
    using ProgramHandle = GlHandle<ProgramTraits>;
    using ShaderHandle = GlHandle<ShaderTraits>;
}

#endif

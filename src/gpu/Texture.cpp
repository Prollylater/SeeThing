#include "Texture.h"

#include <stdexcept>
#include <string>

namespace gpu
{
    namespace
    {
        // Rows are tightly packed or strided, never 4-byte aligned: set the pixel store, restore it after
        class PixelStore
        {
        public:
            PixelStore(GLenum alignment, GLenum rowLength, GLint rowPixels) : m_alignment(alignment), m_rowLength(rowLength)
            {
                glGetIntegerv(m_alignment, &m_oldAlignment);
                glGetIntegerv(m_rowLength, &m_oldRowLength);
                glPixelStorei(m_alignment, 1);
                glPixelStorei(m_rowLength, rowPixels);
            }
            ~PixelStore()
            {
                glPixelStorei(m_alignment, m_oldAlignment);
                glPixelStorei(m_rowLength, m_oldRowLength);
            }
            PixelStore(const PixelStore &) = delete;
            PixelStore &operator=(const PixelStore &) = delete;

        private:
            GLenum m_alignment, m_rowLength;
            GLint m_oldAlignment = 4, m_oldRowLength = 0;
        };
    }

    FormatInfo formatInfo(core::PixelFormat format)
    {
        using core::PixelFormat;
        switch (format)
        {
        case PixelFormat::R8: return {GL_R8, GL_RED, GL_UNSIGNED_BYTE, 1, 1};
        case PixelFormat::RG8: return {GL_RG8, GL_RG, GL_UNSIGNED_BYTE, 2, 1};
        case PixelFormat::RGB8: return {GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, 3, 1};
        case PixelFormat::RGBA8: return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 4, 1};
        case PixelFormat::R32F: return {GL_R32F, GL_RED, GL_FLOAT, 1, 4};
        case PixelFormat::RGB32F: return {GL_RGB32F, GL_RGB, GL_FLOAT, 3, 4};
        case PixelFormat::RGBA32F: return {GL_RGBA32F, GL_RGBA, GL_FLOAT, 4, 4};
        case PixelFormat::R32I: return {GL_R32I, GL_RED_INTEGER, GL_INT, 1, 4};
        case PixelFormat::Unknown: break;
        }
        throw std::invalid_argument("gpu::formatInfo: unknown pixel format");
    }

    Texture::Texture(int width, int height, core::PixelFormat format) : m_w(width), m_h(height), m_format(format)
    {
        if (width <= 0 || height <= 0)
            throw std::invalid_argument("gpu::Texture: empty size");
        const FormatInfo info = formatInfo(format);
        m_tex = TextureHandle::create();
        const GLuint id = m_tex.get();
        glTextureStorage2D(id, 1, info.internal, width, height);
        glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Gray and gray+alpha show as gray, not red
        if (info.channels == 1 && format != core::PixelFormat::R32I)
        {
            const GLint swizzle[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
            glTextureParameteriv(id, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
        }
        else if (info.channels == 2)
        {
            const GLint swizzle[] = {GL_RED, GL_RED, GL_RED, GL_GREEN};
            glTextureParameteriv(id, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
        }
    }

    int Texture::channels() const
    {
        return empty() ? 0 : formatInfo(m_format).channels;
    }

    void Texture::uploadRaw(int x, int y, const void *px, int w, int h, int c, std::ptrdiff_t stride, GLenum type)
    {
        if (empty())
            throw std::logic_error("gpu::Texture::upload on an empty texture");
        const FormatInfo info = formatInfo(m_format);
        if (x < 0 || y < 0 || x + w > m_w || y + h > m_h || c != info.channels || type != info.type)
            throw std::invalid_argument("gpu::Texture::upload: image does not fit the texture (" +
                                        std::to_string(w) + "x" + std::to_string(h) + "x" + std::to_string(c) + " at " +
                                        std::to_string(x) + ", " + std::to_string(y) + ")");
        PixelStore store(GL_UNPACK_ALIGNMENT, GL_UNPACK_ROW_LENGTH, GLint(stride / c));
        glTextureSubImage2D(m_tex.get(), 0, x, y, w, h, info.format, info.type, px);
    }

    void Texture::upload(core::ImageView<const std::uint8_t> image)
    {
        if (image.width() != m_w || image.height() != m_h)
            throw std::invalid_argument("gpu::Texture::upload: image size is not the texture's");
        uploadRaw(0, 0, image.row(0), image.width(), image.height(), image.channels(), image.stride(), GL_UNSIGNED_BYTE);
    }

    void Texture::upload(core::ImageView<const float> image)
    {
        if (image.width() != m_w || image.height() != m_h)
            throw std::invalid_argument("gpu::Texture::upload: image size is not the texture's");
        uploadRaw(0, 0, image.row(0), image.width(), image.height(), image.channels(), image.stride(), GL_FLOAT);
    }

    void Texture::upload(int x, int y, core::ImageView<const std::uint8_t> image)
    {
        if (!image.empty())
            uploadRaw(x, y, image.row(0), image.width(), image.height(), image.channels(), image.stride(), GL_UNSIGNED_BYTE);
    }

    core::Image<std::uint8_t> Texture::download8() const
    {
        if (empty())
            return {};
        const FormatInfo info = formatInfo(m_format);
        if (info.type != GL_UNSIGNED_BYTE)
            throw std::logic_error("gpu::Texture::download8 on a non 8-bit texture");
        core::Image<std::uint8_t> image(m_w, m_h, info.channels);
        PixelStore store(GL_PACK_ALIGNMENT, GL_PACK_ROW_LENGTH, 0);
        glGetTextureImage(m_tex.get(), 0, info.format, info.type, GLsizei(image.size()), image.data());
        return image;
    }

    core::Image<std::uint8_t> Texture::download8(core::Recti rect) const
    {
        if (empty() || rect.empty())
            return {};
        const FormatInfo info = formatInfo(m_format);
        if (info.type != GL_UNSIGNED_BYTE)
            throw std::logic_error("gpu::Texture::download8 on a non 8-bit texture");
        if (rect.x < 0 || rect.y < 0 || rect.right() > m_w || rect.bottom() > m_h)
            throw std::invalid_argument("gpu::Texture::download8: rectangle outside the texture");
        core::Image<std::uint8_t> image(rect.w, rect.h, info.channels);
        PixelStore store(GL_PACK_ALIGNMENT, GL_PACK_ROW_LENGTH, 0);
        glGetTextureSubImage(m_tex.get(), 0, rect.x, rect.y, 0, rect.w, rect.h, 1, info.format, info.type, GLsizei(image.size()), image.data());
        return image;
    }
}

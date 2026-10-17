#ifndef GPU_TEXTURE_H
#define GPU_TEXTURE_H

#include "GlHandle.h"
#include "core/Geometry.h"
#include "core/Image.h"

#include <cstdint>

namespace gpu
{
    struct FormatInfo
    {
        GLenum internal = 0, format = 0, type = 0;
        int channels = 0, bytesPerChannel = 0;
    };
    // Throws on PixelFormat::Unknown
    FormatInfo formatInfo(core::PixelFormat format);

    // 2D texture with immutable storage that knows its own size and format. Row 0 is the image's top row.
    class Texture
    {
    public:
        Texture() = default;
        Texture(int width, int height, core::PixelFormat format);

        template <class T>
        static Texture fromImage(core::ImageView<const T> image)
        {
            Texture tex(image.width(), image.height(), core::pixelFormatOf<T>(image.channels()));
            tex.upload(image);
            return tex;
        }

        // Size and channel count must match the texture
        void upload(core::ImageView<const std::uint8_t> image);
        void upload(core::ImageView<const float> image);
        // The image lands with its top-left pixel at x, y; it must fit inside
        void upload(int x, int y, core::ImageView<const std::uint8_t> image);
        core::Image<std::uint8_t> download8() const;
        core::Image<std::uint8_t> download8(core::Recti rect) const;

        GLuint id() const { return m_tex.get(); }
        int width() const { return m_w; }
        int height() const { return m_h; }
        core::PixelFormat format() const { return m_format; }
        int channels() const;
        bool empty() const { return !m_tex; }

    private:
        void uploadRaw(int x, int y, const void *px, int w, int h, int c, std::ptrdiff_t stride, GLenum type);

        TextureHandle m_tex;
        int m_w = 0, m_h = 0;
        core::PixelFormat m_format = core::PixelFormat::Unknown;
    };
}

#endif

#ifndef CORE_IMAGE_H
#define CORE_IMAGE_H

#include "PixelFormat.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace core
{
    // Non-owning window into interleaved pixels; stride is in elements between rows
    template <class T>
    class ImageView
    {
    public:
        ImageView() = default;
        ImageView(T *px, int w, int h, int c, std::ptrdiff_t stride) : m_px(px), m_w(w), m_h(h), m_c(c), m_stride(stride) {}

        int width() const { return m_w; }
        int height() const { return m_h; }
        int channels() const { return m_c; }
        std::ptrdiff_t stride() const { return m_stride; }
        bool empty() const { return m_px == nullptr || m_w == 0 || m_h == 0; }

        T &operator()(int x, int y, int c = 0) const
        {
            assert(x >= 0 && x < m_w && y >= 0 && y < m_h && c >= 0 && c < m_c);
            return m_px[y * m_stride + std::ptrdiff_t(x) * m_c + c];
        }
        T *row(int y) const { return m_px + y * m_stride; }

        ImageView sub(int x, int y, int w, int h) const
        {
            if (x < 0 || y < 0 || w < 0 || h < 0 || x + w > m_w || y + h > m_h)
                throw std::out_of_range("ImageView::sub outside the view");
            return ImageView(row(y) + std::ptrdiff_t(x) * m_c, w, h, m_c, m_stride);
        }

        operator ImageView<const T>() const
            requires(!std::is_const_v<T>)
        {
            return ImageView<const T>(m_px, m_w, m_h, m_c, m_stride);
        }

    private:
        T *m_px = nullptr;
        int m_w = 0, m_h = 0, m_c = 0;
        std::ptrdiff_t m_stride = 0;
    };

    // Owning, interleaved, row-major pixel buffer: index (y * width + x) * channels + c
    template <class T>
    class Image
    {
    public:
        Image() = default;
        Image(int w, int h, int c, ColorSpace space = ColorSpace::sRGB) : m_space(space)
        {
            if (w < 0 || h < 0 || c < 0)
                throw std::invalid_argument("Image: negative size");
            m_w = w;
            m_h = h;
            m_c = c;
            if (size() > 0)
                m_px = new T[size()](); // () zero-fills
        }
        ~Image() { delete[] m_px; }

        Image(const Image &o) : m_w(o.m_w), m_h(o.m_h), m_c(o.m_c), m_space(o.m_space), m_px(o.size() ? new T[o.size()] : nullptr)
        {
            std::copy(o.m_px, o.m_px + o.size(), m_px);
        }
        Image(Image &&o) noexcept
            : m_w(std::exchange(o.m_w, 0)), m_h(std::exchange(o.m_h, 0)), m_c(std::exchange(o.m_c, 0)),
              m_space(o.m_space), m_px(std::exchange(o.m_px, nullptr)) {}
        // By value: one operator for copy and move, and the old buffer is only freed after the copy succeeded
        Image &operator=(Image o) noexcept
        {
            swap(o);
            return *this;
        }
        void swap(Image &o) noexcept
        {
            std::swap(m_w, o.m_w);
            std::swap(m_h, o.m_h);
            std::swap(m_c, o.m_c);
            std::swap(m_space, o.m_space);
            std::swap(m_px, o.m_px);
        }

        std::size_t size() const { return std::size_t(m_w) * m_h * m_c; }
        bool empty() const { return m_px == nullptr; }
        int width() const { return m_w; }
        int height() const { return m_h; }
        int channels() const { return m_c; }
        PixelFormat format() const { return pixelFormatOf<T>(m_c); }
        ColorSpace colorSpace() const { return m_space; }
        void setColorSpace(ColorSpace space) { m_space = space; }

        T *data() { return m_px; }
        const T *data() const { return m_px; }
        T *row(int y) { return m_px + std::size_t(y) * m_w * m_c; }
        const T *row(int y) const { return m_px + std::size_t(y) * m_w * m_c; }

        // Unchecked in release builds: for inner loops
        T &operator()(int x, int y, int c = 0)
        {
            assert(inside(x, y, c));
            return m_px[index(x, y, c)];
        }
        const T &operator()(int x, int y, int c = 0) const
        {
            assert(inside(x, y, c));
            return m_px[index(x, y, c)];
        }
        T &at(int x, int y, int c = 0)
        {
            if (!inside(x, y, c))
                throw std::out_of_range("Image::at outside the image");
            return m_px[index(x, y, c)];
        }
        const T &at(int x, int y, int c = 0) const { return const_cast<Image &>(*this).at(x, y, c); }

        bool inside(int x, int y, int c = 0) const { return x >= 0 && x < m_w && y >= 0 && y < m_h && c >= 0 && c < m_c; }

        void fill(T value) { std::fill(m_px, m_px + size(), value); }

        ImageView<T> view() { return ImageView<T>(m_px, m_w, m_h, m_c, std::ptrdiff_t(m_w) * m_c); }
        ImageView<const T> view() const { return ImageView<const T>(m_px, m_w, m_h, m_c, std::ptrdiff_t(m_w) * m_c); }
        operator ImageView<const T>() const { return view(); }

    private:
        std::size_t index(int x, int y, int c) const { return (std::size_t(y) * m_w + x) * m_c + c; }

        int m_w = 0, m_h = 0, m_c = 0;
        ColorSpace m_space = ColorSpace::sRGB;
        T *m_px = nullptr; // owned
    };

    template <class T>
    void swap(Image<T> &a, Image<T> &b) noexcept { a.swap(b); }
}

#endif

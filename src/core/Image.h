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
        ImageView(T *px, int width, int height, int channels, std::ptrdiff_t stride) : m_pixels(px), m_width(width), m_height(height), m_channels(channels), m_stride(stride) {}

        int width() const { return m_width; }
        int height() const { return m_height; }
        int channels() const { return m_channels; }
        std::ptrdiff_t stride() const { return m_stride; }
        bool empty() const { return m_pixels == nullptr || m_width == 0 || m_height == 0; }

        T &operator()(int x, int y, int c = 0) const
        {
            assert(x >= 0 && x < m_width && y >= 0 && y < m_height && c >= 0 && c < m_channels);
            return m_pixels[y * m_stride + std::ptrdiff_t(x) * m_channels + c];
        }
        T *row(int y) const { return m_pixels + y * m_stride; }

        ImageView sub(int x, int y, int width, int height) const
        {
            if (x < 0 || y < 0 || width < 0 || height < 0 || x + width > m_width || y + height > m_height)
                throw std::out_of_range("ImageView::sub outside the view");
            return ImageView(row(y) + std::ptrdiff_t(x) * m_channels, width, height, m_channels, m_stride);
        }

        operator ImageView<const T>() const
            requires(!std::is_const_v<T>)
        {
            return ImageView<const T>(m_pixels, m_width, m_height, m_channels, m_stride);
        }

    private:
        T *m_pixels = nullptr;
        int m_width = 0, m_height = 0, m_channels = 0;
        std::ptrdiff_t m_stride = 0;
    };

    // Owning, interleaved, row-major pixel buffer: index (y * width + x) * channels + c
    template <class T>
    class Image
    {
    public:
        Image() = default;
        Image(int width, int height, int channels, ColorSpace space = ColorSpace::sRGB) : m_space(space)
        {
            if (width < 0 || height < 0 || channels < 0)
                throw std::invalid_argument("Image: negative size");
            m_width = width;
            m_height = height;
            m_channels = channels;
            if (size() > 0)
                m_pixels = new T[size()](); // () zero-fills
        }
        ~Image() { delete[] m_pixels; }

        Image(const Image &other) : m_width(other.m_width), m_height(other.m_height), m_channels(other.m_channels), m_space(other.m_space), m_pixels(other.size() ? new T[other.size()] : nullptr)
        {
            std::copy(other.m_pixels, other.m_pixels + other.size(), m_pixels);
        }
        Image(Image &&other) noexcept
            : m_width(std::exchange(other.m_width, 0)), m_height(std::exchange(other.m_height, 0)), m_channels(std::exchange(other.m_channels, 0)),
              m_space(other.m_space), m_pixels(std::exchange(other.m_pixels, nullptr)) {}
        // By value: one operator for copy and move, and the old buffer is only freed after the copy succeeded
        Image &operator=(Image other) noexcept
        {
            swap(other);
            return *this;
        }
        void swap(Image &other) noexcept
        {
            std::swap(m_width, other.m_width);
            std::swap(m_height, other.m_height);
            std::swap(m_channels, other.m_channels);
            std::swap(m_space, other.m_space);
            std::swap(m_pixels, other.m_pixels);
        }

        std::size_t size() const { return std::size_t(m_width) * m_height * m_channels; }
        bool empty() const { return m_pixels == nullptr; }
        int width() const { return m_width; }
        int height() const { return m_height; }
        int channels() const { return m_channels; }
        PixelFormat format() const { return pixelFormatOf<T>(m_channels); }
        ColorSpace colorSpace() const { return m_space; }
        void setColorSpace(ColorSpace space) { m_space = space; }

        T *data() { return m_pixels; }
        const T *data() const { return m_pixels; }
        T *row(int y) { return m_pixels + std::size_t(y) * m_width * m_channels; }
        const T *row(int y) const { return m_pixels + std::size_t(y) * m_width * m_channels; }

        // Unchecked in release builds: for inner loops
        T &operator()(int x, int y, int c = 0)
        {
            assert(inside(x, y, c));
            return m_pixels[index(x, y, c)];
        }
        const T &operator()(int x, int y, int c = 0) const
        {
            assert(inside(x, y, c));
            return m_pixels[index(x, y, c)];
        }
        T &at(int x, int y, int c = 0)
        {
            if (!inside(x, y, c))
                throw std::out_of_range("Image::at outside the image");
            return m_pixels[index(x, y, c)];
        }
        const T &at(int x, int y, int c = 0) const { return const_cast<Image &>(*this).at(x, y, c); }

        bool inside(int x, int y, int c = 0) const { return x >= 0 && x < m_width && y >= 0 && y < m_height && c >= 0 && c < m_channels; }

        void fill(T value) { std::fill(m_pixels, m_pixels + size(), value); }

        ImageView<T> view() { return ImageView<T>(m_pixels, m_width, m_height, m_channels, std::ptrdiff_t(m_width) * m_channels); }
        ImageView<const T> view() const { return ImageView<const T>(m_pixels, m_width, m_height, m_channels, std::ptrdiff_t(m_width) * m_channels); }
        operator ImageView<const T>() const { return view(); }

    private:
        std::size_t index(int x, int y, int c) const { return (std::size_t(y) * m_width + x) * m_channels + c; }

        int m_width = 0, m_height = 0, m_channels = 0;
        ColorSpace m_space = ColorSpace::sRGB;
        T *m_pixels = nullptr; // owned
    };

    template <class T>
    void swap(Image<T> &a, Image<T> &b) noexcept { a.swap(b); }
}

#endif

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
        ImageView(T *px, int w, int h, int c, std::ptrdiff_t stride) : px_(px), w_(w), h_(h), c_(c), stride_(stride) {}

        int width() const { return w_; }
        int height() const { return h_; }
        int channels() const { return c_; }
        std::ptrdiff_t stride() const { return stride_; }
        bool empty() const { return px_ == nullptr || w_ == 0 || h_ == 0; }

        T &operator()(int x, int y, int c = 0) const
        {
            assert(x >= 0 && x < w_ && y >= 0 && y < h_ && c >= 0 && c < c_);
            return px_[y * stride_ + std::ptrdiff_t(x) * c_ + c];
        }
        T *row(int y) const { return px_ + y * stride_; }

        ImageView sub(int x, int y, int w, int h) const
        {
            if (x < 0 || y < 0 || w < 0 || h < 0 || x + w > w_ || y + h > h_)
                throw std::out_of_range("ImageView::sub outside the view");
            return ImageView(row(y) + std::ptrdiff_t(x) * c_, w, h, c_, stride_);
        }

        operator ImageView<const T>() const
            requires(!std::is_const_v<T>)
        {
            return ImageView<const T>(px_, w_, h_, c_, stride_);
        }

    private:
        T *px_ = nullptr;
        int w_ = 0, h_ = 0, c_ = 0;
        std::ptrdiff_t stride_ = 0;
    };

    // Owning, interleaved, row-major pixel buffer: index (y * width + x) * channels + c
    template <class T>
    class Image
    {
    public:
        Image() = default;
        Image(int w, int h, int c, ColorSpace space = ColorSpace::sRGB) : space_(space)
        {
            if (w < 0 || h < 0 || c < 0)
                throw std::invalid_argument("Image: negative size");
            w_ = w;
            h_ = h;
            c_ = c;
            if (size() > 0)
                px_ = new T[size()](); // () zero-fills
        }
        ~Image() { delete[] px_; }

        Image(const Image &o) : w_(o.w_), h_(o.h_), c_(o.c_), space_(o.space_), px_(o.size() ? new T[o.size()] : nullptr)
        {
            std::copy(o.px_, o.px_ + o.size(), px_);
        }
        Image(Image &&o) noexcept
            : w_(std::exchange(o.w_, 0)), h_(std::exchange(o.h_, 0)), c_(std::exchange(o.c_, 0)),
              space_(o.space_), px_(std::exchange(o.px_, nullptr)) {}
        // By value: one operator for copy and move, and the old buffer is only freed after the copy succeeded
        Image &operator=(Image o) noexcept
        {
            swap(o);
            return *this;
        }
        void swap(Image &o) noexcept
        {
            std::swap(w_, o.w_);
            std::swap(h_, o.h_);
            std::swap(c_, o.c_);
            std::swap(space_, o.space_);
            std::swap(px_, o.px_);
        }

        std::size_t size() const { return std::size_t(w_) * h_ * c_; }
        bool empty() const { return px_ == nullptr; }
        int width() const { return w_; }
        int height() const { return h_; }
        int channels() const { return c_; }
        PixelFormat format() const { return pixelFormatOf<T>(c_); }
        ColorSpace colorSpace() const { return space_; }
        void setColorSpace(ColorSpace space) { space_ = space; }

        T *data() { return px_; }
        const T *data() const { return px_; }
        T *row(int y) { return px_ + std::size_t(y) * w_ * c_; }
        const T *row(int y) const { return px_ + std::size_t(y) * w_ * c_; }

        // Unchecked in release builds: for inner loops
        T &operator()(int x, int y, int c = 0)
        {
            assert(inside(x, y, c));
            return px_[index(x, y, c)];
        }
        const T &operator()(int x, int y, int c = 0) const
        {
            assert(inside(x, y, c));
            return px_[index(x, y, c)];
        }
        T &at(int x, int y, int c = 0)
        {
            if (!inside(x, y, c))
                throw std::out_of_range("Image::at outside the image");
            return px_[index(x, y, c)];
        }
        const T &at(int x, int y, int c = 0) const { return const_cast<Image &>(*this).at(x, y, c); }

        bool inside(int x, int y, int c = 0) const { return x >= 0 && x < w_ && y >= 0 && y < h_ && c >= 0 && c < c_; }

        void fill(T value) { std::fill(px_, px_ + size(), value); }

        ImageView<T> view() { return ImageView<T>(px_, w_, h_, c_, std::ptrdiff_t(w_) * c_); }
        ImageView<const T> view() const { return ImageView<const T>(px_, w_, h_, c_, std::ptrdiff_t(w_) * c_); }
        operator ImageView<const T>() const { return view(); }

    private:
        std::size_t index(int x, int y, int c) const { return (std::size_t(y) * w_ + x) * c_ + c; }

        int w_ = 0, h_ = 0, c_ = 0;
        ColorSpace space_ = ColorSpace::sRGB;
        T *px_ = nullptr; // owned
    };

    template <class T>
    void swap(Image<T> &a, Image<T> &b) noexcept { a.swap(b); }
}

#endif

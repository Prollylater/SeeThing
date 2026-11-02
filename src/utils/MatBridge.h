#ifndef MATBRIDGE_H
#define MATBRIDGE_H

#include "Mat.h"
#include "core/Image.h"

#include <algorithm>
#include <cstdint>

inline core::ImageView<const std::uint8_t> viewOf(const Mat<std::uint8_t> &m)
{
    return {m.getData(), m.getCols(), m.getRows(), m.getChannels(), std::ptrdiff_t(m.getCols()) * m.getChannels()};
}

inline Mat<std::uint8_t> toMat(core::ImageView<const std::uint8_t> v)
{
    if (v.empty())
        return Mat<std::uint8_t>();
    Mat<std::uint8_t> m(v.height(), v.width(), v.channels());
    std::uint8_t *dst = m.at(0, 0);
    const std::size_t rowSize = std::size_t(v.width()) * v.channels();
    for (int y = 0; y < v.height(); ++y)
        std::copy(v.row(y), v.row(y) + rowSize, dst + y * rowSize);
    return m;
}

#endif

#ifndef LAYER_H
#define LAYER_H
#include <cstdint>
#include <utility>
#include "core/Image.h"

class Layer
{
public:
    Layer() = default;
    explicit Layer(core::Image<uint8_t> img) : image(std::move(img)) {}

    int getWidth() const { return image.width(); }
    int getHeight() const { return image.height(); }
    const core::Image<uint8_t> &getImage() const { return image; }

    // TODO Make this private and create function handling mangagment
    core::Image<uint8_t> image;
};

#endif

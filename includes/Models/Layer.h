#ifndef LAYER_H
#define LAYER_H
#include <cstdint>
#include <utility>
#include "Mat.h"

// You can access most thing here
//  Apply constructor and so
// FIRST USE LAYER TO LIMIT THE NECESSITY OF PASSING THING LIKE WIDTH AND HEIGT
class Layer
{
public:
    Layer() = default;
    Layer(const Mat<uint8_t> &img) : image(img) {}
    // img has a name here, so it must be moved explicitly
    Layer(Mat<uint8_t> &&img) : image(std::move(img)) {}

    // Methods to get and set pixel data
    // Color getPixel(int x, int y) const;
    // void setPixel(int x, int y);
    int getSize()
    {
        return image.getCols() * image.getRows();
    }

    int getWidth()
    {
        return image.getCols();
    }

    int getHeight()
    {
        return image.getRows();
    }

    Mat<uint8_t> &getImage()
    {

        return image;
    }

    // TODO Make this private and create function handling mangagment
    // private
    Mat<uint8_t> image;
};

#endif

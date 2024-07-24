#include "Mat.h"

// Apply constructor and so
class Layer
{
public:
    Layer(Mat<uint8_t> &img)
    {
        image = &img;
    }

    // Methods to get and set pixel data
    // Color getPixel(int x, int y) const;
    // void setPixel(int x, int y);
    int getSize()
    {
        if (image==nullptr)
        {
            return 0;
        }
        return image->getCols() * image->getRows();
    }

     int getWidth()
    {
        if (image==nullptr)
        {
            return 0;
        }
        return image->getCols();
    }


  int getHeight()
    {
        if (image==nullptr)
        {
            return 0;
        }
        return image->getRows();
    }

    // TODO Make this private and create function handling mangagment
    // private
    Mat<uint8_t> *image;
};

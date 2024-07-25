#include "Mat.h"

//You can access most thing here 
// Apply constructor and so
//FIRST USE LAYER TO LIMIT THE NECESSITY OF PASSING THING LIKE WIDTH AND HEIGT
class Layer
{
public:
    Layer(Mat<uint8_t> &img)
    {
        image = img;
    }
    Layer(Mat<uint8_t> &&img)
    {
        image = img;
    }


    // Methods to get and set pixel data
    // Color getPixel(int x, int y) const;
    // void setPixel(int x, int y);
    int getSize()
    {
        if (&image==nullptr)
        {
            return 0;
        }
        return image.getCols() * image.getRows();
    }

     int getWidth()
    {
        if (&image==nullptr)
        {
            return 0;
        }
        return image.getCols();
    }


  int getHeight()
    {
        if (&image==nullptr)
        {
            return 0;
        }
        return image.getRows();
    }


   Mat<uint8_t>& getImage()
    {
        
        return image;
    }


    // TODO Make this private and create function handling mangagment
    // private
    Mat<uint8_t> image;
};

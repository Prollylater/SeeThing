
#include "Mat.h"


// Gamma Correction value
#define Gamma_C_V 0.04045
#define Gamma_C_D 12.92
// https://docs.opencv.org/3.4/de/d25/imgproc_color_conversions.html
// https://www.easyrgb.com/en/math.php
//  For RGB
float gamma_correctLab(double c)
{
    if (c <= Gamma_C_V)
    {
        return c / Gamma_C_D;
    }
    else
    {
        return std::pow((c + 0.055) / 1.055, 2.4);
    }
}

// For XYZ
float functiontt(float c, float a , float b )
{
    if (c <= 0.008856)
    {
        return a * c + b;
    }
    else
    {
        return std::pow(c, 1 / 3.f);
    }
}


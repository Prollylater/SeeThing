#include "Helper.h"

Mat<int> normalizeHist(Mat<int> &tonormalize, int minrange, int maxrange)
{
    // TODO, open Mat to operation with differnt type.
    int min, max;

    Mat<int> hist(tonormalize);
    hist.minMaxLoc(min, max);
    hist = (hist - min) * ((maxrange - minrange) / (max - min)) + minrange;

    return hist;
}

// TODO, convert to template
//  Seriously recheck this
Mat<int> stretchHist(const Mat<int> &img, int b, int a)
{
    // Img is an hsitogram
    Mat<int> newimg = Mat<int>(img.getRows(), img.getCols(), img.getChannels());
    int min, max;

    img.minMaxLoc(min, max);

    for (int i = 0; i < img.getRows(); i++)
    {
        for (int j = 0; j < img.getCols(); ++j)
        {
            for (int c = 0; j < img.getChannels(); ++c)
            {

                if (max == 255 && min == 0)
                {
                    return newimg;
                }
                else
                {
                    newimg.atChannel(i, j, c) = (b - a) * (img.atChannel(i, j, c) - min) / (max - min) + a;
                }
            }
        }
    }
    return newimg;
}

int findPeak(const Mat<int> &hist)
{
    int peakindex = 0;
    int maxFrequency = 0;

    for (int j = 0; j < hist.getRows(); j++)
    {
        for (int c = 0; c < hist.getChannels(); c++)
        { // May not be necessary
            if (hist.atChannel(j, 0, c) > maxFrequency)
            {
                maxFrequency = hist.atChannel(j, 0, c);
                peakindex = j;
            }
        }
    }
    return peakindex;
}

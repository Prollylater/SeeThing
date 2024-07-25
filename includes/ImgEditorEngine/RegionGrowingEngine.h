#ifndef RG_H
#define RG_H
#include "StructParam.h"
#include "RegionGrowingAlg.h"
// Module in charge of region growing
//  TODO Transfer relevant part to cpp and add to cmake

class RGEngine; 

namespace appobj
{
    // Create an engine that withold
    extern RGEngine rgengine;
}

class RGEngine
{
private:
    std::vector<Region> regions;

public:
     Mat<uint8_t>  dispatch(const SlicParameter &exec_param, const Mat<uint8_t> &image, int coloration)
    {

        SlicAlgorithm(regions, image, exec_param);
        // TODO LEt the choice to have different color
        //output(coloration,image);
        return composeSegRandCol(image, regions);
    }

    void dispatch(const SeedRGParameter &exec_param, const Mat<uint8_t> &image)
    {
    }

    Mat<uint8_t> output(int color, const Mat<uint8_t> &image)
    {
        switch (color)
        {
        case 0:
            return composeSegRandCol(image, regions);
        case 1:
            return composeSegMeanColor(image, regions);
        case 2:
            return composeSegMean(image, regions);
        }
    }
};

#endif
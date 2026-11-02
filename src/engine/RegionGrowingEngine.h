#ifndef RG_H
#define RG_H
#include "StructParam.h"
#include "RegionGrowingAlg.h"
// Module in charge of region growing
// TODO Transfer relevant part to cpp and add to cmake
// TODO add benchmark capacity to output to screen
// TODO More efficieent way to store it in memory
class RGEngine
{
private:
    // Hold last streated region in
    // Needed to implement quick modification
    // TODO ; regions is cleared by the region growing algorithm.
    // WHich is pretty bad
    std::vector<Region> regions;

public:
    // roi: nullptr for the whole image, else pixels under 128 are left out (and black in the output)
    Mat<uint8_t> dispatch(const SlicParameter &exec_param, const Mat<uint8_t> &image, int coloration, const Mat<uint8_t> *roi = nullptr)
    {

        SlicAlgorithm(regions, image, exec_param, roi);
        return output(coloration, image);
    }

    Mat<uint8_t> dispatch(const SeedRGParameter &exec_param, const Mat<uint8_t> &image, int coloration, const Mat<uint8_t> *roi = nullptr)
    {

        std::queue<std::shared_ptr<Pixl>> seedlist;
        // if( m_random){}
        if (exec_param.m_random)
        {
            std::cout << "Germs selection" << std::endl;

            if (exec_param.m_divide)
            {
                seedlist = putGermsDivide(image, regions, exec_param.m_seednb, roi);
            }
            else
            {

                seedlist = putGerms(image, regions, exec_param.m_seednb, roi);
            }
        }
        else
        {
            seedlist = putGermsAt(image, regions, exec_param.m_points, roi);
        }
        std::cout << "RegionALg" << std::endl;

        Mat<uint32_t> visited(image.getRows(), image.getCols(), 1); // Doesn't need to exist here

        if (!exec_param.m_rg_type)
        {
            regionGrowing3c(image, seedlist, exec_param.m_threshold, visited, regions, exec_param.m_comp_algorithm, roi);
        }
        else
        {
            adaptiveRegionGrowing(image, seedlist, exec_param.m_threshold, visited, regions, exec_param.m_adaptation, roi);
        }

        std::cout << "Fusing" << std::endl;

        fuseRegions(regions, exec_param.m_threshold);

        return output(coloration, image);
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
        default:
            return composeSegMeanColor(image, regions);
        }
    }
};

#endif
#ifndef RG_GRW_H
#define RG_GRW_H
#include <queue>
#include "Regions.h"
#include "Imloader.h"
#include "StructParam.h"

#include <random>

// TODO, Check which template are really useful

int moduloc(int dividend, int divisor);

// TODO revoir
//  Pose de germes
template <typename T>
std::shared_ptr<Pixl> putGerm(const Mat<T> &img, int x, int y, std::vector<Region> &regions)
{
    std::queue<std::shared_ptr<Pixl>> seedlist; // Stores the seeds
    // Add a new region
    regions.push_back(Region());

    std::shared_ptr<Pixl> pix(new Pixl(x, y, regions.size()));
    seedlist.push(pix);
    regions[pix->getId()].addGerm(pix, img);
    return pix;
}

// Pose des germes aléatoire
// TODO Also, put a guard in the app
template <typename T>
std::queue<std::shared_ptr<Pixl>> putGerms(const Mat<T> &img, std::vector<Region> &regions, int germsquant)
{
    regions.clear();
    regions.shrink_to_fit();

    if (germsquant > (img.getRows() * img.getCols()))
    {
        // Or return
        germsquant = (img.getRows() * img.getCols()) - 1;
        std::cout << "Number of seed too big for current image" << std::endl;
    }
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());

    std::uniform_int_distribution<int> dist_rows(0, img.getRows() - 1);
    std::uniform_int_distribution<int> dist_cols(0, img.getCols() - 1);

    std::queue<std::shared_ptr<Pixl>> seedlist; // Stores the seeds
    for (int i = 0; i < germsquant; i++)
    {
        regions.push_back(Region());
    }
    for (int i = 0; i < germsquant; i++)
    {
        int x = dist_cols(generator); // generate a random number for x
        int y = dist_rows(generator);
        std::shared_ptr<Pixl> pix(new Pixl(x, y, i));

        seedlist.push(pix);
        regions[i].addGerm(pix, img);
    }

    return seedlist;
}

// Pose des germes aléatoire après division de l'image
template <typename T>
std::queue<std::shared_ptr<Pixl>> putGermsDivide(const Mat<T> &img, std::vector<Region> &regions, int n)
{
    regions.clear();
    regions.shrink_to_fit();

    std::srand(static_cast<unsigned int>(std::time(0)));

    // Get a number of germ through the commandline
    std::queue<std::shared_ptr<Pixl>> seedlist; // Stores the seeds
    int index = 0;
    int germquant = n * n;
    // Divide in n part on each side
    int partheight = img.getRows() / n;
    int partwidth = img.getCols() / n;

    if (partheight == 0 || partwidth == 0)
    { // partwidth  a des résultats intèressants
        // Set the part size to a prechosen value
        partheight = 4;
        partwidth = 4;
        // Since partsize don't depend of germquant there, germquant must depend from partsize
        germquant = (img.getRows() * img.getCols()) / (partheight * partwidth);
    }

    // allocate as much region as there is seed

    for (int i = 0; i < germquant; ++i)
    {
        regions.push_back(Region());
    }

    // TODO, handle neighbour here
    while (index < germquant)
    {

        for (int i = 0; i < n && index < germquant; ++i)
        {
            for (int j = 0; j < n && index < germquant; ++j)
            {
                int startrow = i * partheight;
                int endrow = ((i + 1) * partheight) - 1;
                int startcol = j * partwidth;
                int endcol = ((j + 1) * partwidth) - 1;

                // Extract the current part from the image
                int y = startcol + std::rand() % (endcol - startcol + 1); // generate a random number for x

                int x = startrow + std::rand() % (endrow - startrow + 1);

                std::shared_ptr<Pixl> pix(new Pixl(x, y, index));
                seedlist.push(pix);
                regions[index].addGerm(pix, img);
                ++index;
            }
        }
    }

    return seedlist;
}

template <typename T>
// T always U char ?
// TODO UPDATE, DOn't work currentyl
void medianBlurCustom(const Mat<T> &img, Mat<T> &dst)
{
    dst = img.clone();
    int sizeKer = 3;
    int edgex = sizeKer / 2;
    int edgey = sizeKer / 2;
    std::vector<std::vector<Vec<T>>> values(3, std::vector<Vec<T>>(sizeKer * sizeKer));

    for (int x = edgex; x < img.getCols() - edgex; x++)
    {
        for (int y = edgey; y < img.getRows() - edgey; y++)
        {
            for (int c = 0; c < 3; c++)
            {

                int i = 0;
                for (int fx = 0; fx <= sizeKer; fx++)
                {
                    for (int fy = 0; fy <= sizeKer; fy++)
                    {
                        values[c][i] = img.atVec(y + fy - edgey, x + fx - edgex)[c];
                        i++;
                    }
                }

                // Sort the window
                std::sort(values.begin(), values.end());
            }
            // Set the median value as the output pixel value
            for (int c = 0; c < 3; c++)
            {
                //  dst.atChannel(y, x,c) = values[c][(sizeKer * sizeKer) / 2];
            }
        }
    }
}

// TODO  Change place
//  Croissance de régons autour des germes(résultat plusieurs régions)
//  Fusion de régions adjacentes semblablec=
//  Load an image with some basic tratment applied
/*
Mat<T> imgTreat(std::string path, int treat)
{
    cv::Mat img = cv::imread(path, 1);

    // Check for failure
    if (img.empty())
    {
        std::cerr << "Could not find the image" << std::endl;
        return cv::Mat::zeros(2, 2, CV_8UC3);
    }

    cv::Mat kernel;

    cv::Mat convolved;

    // convolution
    if (img.getChannels() == 3 && treat == 3)
    {
        // Gaussian Blur
        kernel = createMatNxN(3, 1.0 / 16, 2.0 / 16, 1.0 / 16, 2.0 / 16, 4.0 / 16, 2.0 / 16, 1.0 / 16, 2.0 / 16, 1.0 / 16);
        convolved = applyFilterConv3C3(img, kernel);
        return convolved;
    }
    else if (img.getChannels() == 3 && treat == 2)
    {
        medianBlurCustom(img, convolved);
        return convolved;
    }
    else if (img.getChannels() == 1 && treat == 1)
    {
        // Mean blur
        kernel = createMatNxN(3, 1.0 / 9, 1.0 / 9, 1.0 / 9, 1.0 / 9, 1.0 / 9, 1.0 / 9, 1.0 / 9, 1.0 / 9, 1.0 / 9);
        convolved = applyFilterConv3(img, kernel);
        return convolved;
    }

    return img;
}
*/
//
// Seeded region growing
//
constexpr int RG_FUNCT_NB = 4;
static rgsCompFuncVar rgs_comp_func_var[RG_FUNCT_NB] = {
    isSimilarIntensity3c, isSimilarIntensity3cGlobal,
    isSimilarLuminance3cGlobal, isSimilarEucDist};

// TODO Visited might as well  be initalized here
// Execute eRegionGrowing algorithm with 1c SImilarity function
template <typename T>
void regionGrowing1c(const Mat<T> &image, std::queue<std::shared_ptr<Pixl>> &pixelQueue,
                     const float threshold, Mat<uint32_t> &visited, std::vector<Region> &regions, int compfunc = 0)
{
    visited.setTo(std::numeric_limits<uint32_t>::max());
    uint32_t valctrl = std::numeric_limits<uint32_t>::max();

    rgsCompFuncVar exec_funct;
    if (compfunc < RG_FUNCT_NB)
    {
        exec_funct = rgs_comp_func_var[compfunc];
    }
    else
    {
        exec_funct = rgs_comp_func_var[0];
    }
    while (!pixelQueue.empty())
    {
        std::shared_ptr<Pixl> currPixel = pixelQueue.front();
        pixelQueue.pop();
        visited.atChannel(currPixel->y, currPixel->x, 0) = currPixel->getId();
        // Precalculate BOundary
        int startX = std::max(currPixel->x - 1, 0);
        int endX = std::min(currPixel->x + 1, image.getCols() - 1);
        int startY = std::max(currPixel->y - 1, 0);
        int endY = std::min(currPixel->y + 1, image.getRows() - 1);

        // Add the seed pixel to the region
        for (int i = startX; i <= endX; ++i)
        {
            for (int j = startY; j <= endY; ++j)
            {
                // First visit of a pixel
                if ((visited.atChannel(j, i, 0)) == valctrl)
                {
                    std::shared_ptr<Pixl> neighbor(new Pixl(i, j, currPixel->getId()));
                    bool similar = false;
                    // TODO Push this part to a  comparison function
                    // Can finally use iinitialization inside if properly
                    if (auto funct = std::get_if<CompPixPix>(&exec_funct))
                    {
                        similar = (*funct)(image, currPixel, (neighbor), threshold);
                    }
                    if (auto funct = std::get_if<CompPixRegions>(&exec_funct))
                    {
                        similar = (*funct)(image, regions[currPixel->getId()], neighbor, threshold);
                    }
                    if (similar
                        // isSimilarIntensity1c(image, currPixel, (neighbor), threshold)
                    )
                    {
                        pixelQueue.push(neighbor);
                        // Add the neighboring pixel to the region
                        visited.atChannel(neighbor->y, neighbor->x, 0) = currPixel->getId();
                        regions[currPixel->getId()].addGerm(neighbor, image);
                    }
                }
                // Pixel already visited, we decide if we want to add him as neighbor
                else
                {
                    if ((visited.atChannel(j, i, 0) != visited.atChannel(currPixel->y, currPixel->x, 0)))
                    {
                        unsigned int regionid = visited.atChannel(j, i, 0);
                        // Add as adjacent regions
                        regions[currPixel->getId()].addAdjacentRegion(&(regions[regionid]));
                        regions[regionid].addAdjacentRegion(&(regions[currPixel->getId()]));
                    }
                }
            }
        }
    }
}

// Execute eRegionGrowing algorithm with 3c SImilarity function
template <typename T>
void regionGrowing3c(const Mat<T> &image, std::queue<std::shared_ptr<Pixl>> &pixelQueue,
                     const float threshold, Mat<uint32_t> &visited, std::vector<Region> &regions, int compfunc = 0)
{
    visited.setTo(std::numeric_limits<uint32_t>::max());
    uint32_t valctrl = std::numeric_limits<uint32_t>::max();
    rgsCompFuncVar exec_funct;
    if (compfunc < RG_FUNCT_NB)
    {
        exec_funct = rgs_comp_func_var[compfunc];
    }
    else
    {
        exec_funct = rgs_comp_func_var[0];
    }
    while (!pixelQueue.empty())
    {
        std::shared_ptr<Pixl> currPixel = pixelQueue.front();
        pixelQueue.pop();

        visited.atChannel(currPixel->y, currPixel->x, 0) = currPixel->getId();
        // Precalculate BOundary
        int startX = std::max(currPixel->x - 1, 0);
        int endX = std::min(currPixel->x + 1, image.getCols() - 1);
        int startY = std::max(currPixel->y - 1, 0);
        int endY = std::min(currPixel->y + 1, image.getRows() - 1);

        // Add the seed pixel to the region
        for (int i = startX; i <= endX; ++i)
        {

            for (int j = startY; j <= endY; ++j)
            {

                // First visit of a pixel
                if ((visited.atChannel(j, i, 0)) == valctrl)
                {

                    std::shared_ptr<Pixl> neighbor(new Pixl(i, j, currPixel->getId()));
                    bool similar = false;
                    // Resolve correct comparison function
                    // Can finally use iinitialization inside if properly
                    if (auto funct = std::get_if<CompPixPix>(&exec_funct))

                    {
                        similar = (*funct)(image, currPixel, (neighbor), threshold);
                    }
                    if (auto funct = std::get_if<CompPixRegions>(&exec_funct))
                    {
                        similar = (*funct)(image, regions[currPixel->getId()], neighbor, threshold);
                    }
                    if (similar)
                    {
                        // Add the neighboring pixel to the region

                        pixelQueue.push(neighbor);

                        // Add the neighboring pixel to the region
                        visited.atChannel(neighbor->y, neighbor->x, 0) = static_cast<u_int32_t>(currPixel->getId());
                        regions[currPixel->getId()].addGerm(neighbor, image);
                    }
                }
                else
                {

                    // TODO Remnant from in Region growing fusion ?
                    // in if : && regions[static_cast<int>(visited.at<int>(i, j)) - 1].getSizePix() > 0)

                    if ((visited.atChannel(j, i, 0) != visited.atChannel(currPixel->y, currPixel->x, 0)))
                    {
                        uint32_t regionid = visited.atChannel(j, i, 0);
                        // Add as adjacent regions
                        regions[currPixel->getId()].addAdjacentRegion(&(regions[regionid]));
                        regions[regionid].addAdjacentRegion(&(regions[currPixel->getId()]));
                    }
                }
            }
        }
    }
}

// Loop through the image to find adjacent pixel
template <typename T>
void findRegAdjacency(const Mat<T> &image, std::vector<Region> &regions,
                      const Mat<uint32_t> &visited, const float threshold)
{

    Mat<uint32_t> used(visited);
    for (int i = 0; i < image.getCols(); ++i)
    {
        for (int j = 0; j < image.getRows(); ++j)
        {
            if (!(used.atChannel(i, j, 0) > 0))
            {
                continue;
            }

            int startX = std::max(i - 1, 0);
            int endX = std::min(i + 1, image.getCols() - 1);
            int startY = std::max(j - 1, 0);
            int endY = std::min(j + 1, image.getRows() - 1);

            for (int x = startX; x <= endX; ++x)
            {
                for (int y = startY; y <= endY; ++y)
                { // TODO,
                    if ((used.atChannel(y, x, 0) > 0) && (used.atChannel(y, x, 0) != used.atChannel(j, i, 0)) &&
                        isSimilarMeanAverage(regions[used.atChannel(y, x, 0)], regions[used.atChannel(j, i, 0)], threshold))
                    {
                        int regionid = used.atChannel(y, x, 0);
                        // Add as adjacent regions
                        regions[used.atChannel(y, x, 0)].addAdjacentRegion(&(regions[used.atChannel(j, i, 0)]));
                        regions[used.atChannel(j, i, 0)].addAdjacentRegion(&(regions[used.atChannel(y, x, 0)]));
                    }
                }
            }
        }
    }
}

template <typename T>
bool similarityFunction(const Mat<T> &image, std::shared_ptr<Pixl> currentPixel, const Region &region, std::shared_ptr<Pixl> neighbor, float threshold, bool switchparam)
{
    if (!switchparam)
    {
        return isSimilarIntensity3c(image, currentPixel, neighbor, threshold);
    }
    else
    {
        return isSimilarIntensity3cGlobal(image, region, neighbor, threshold);
    }
    // OR
    // return isSimidlarLuminance3cGlobal(image, regions[currentPixel->getId()], neighbor, threshold);
};

// TODO, add proper mangament of the functions
template <typename T>
void adaptiveRegionGrowing(const Mat<T> &image, std::queue<std::shared_ptr<Pixl>> &pixelQueue,
                           const float threshold, Mat<unsigned int> &visited, std::vector<Region> &regions, int adaptative)
{

    visited.setTo(std::numeric_limits<uint32_t>::max());
    uint32_t valctrl = std::numeric_limits<uint32_t>::max();

    int imgsize = image.getRows() * image.getCols();
    int currentsegmentationsize = pixelQueue.size();
    int switchpercentage = (imgsize * adaptative) / 100;
    bool switchparam = false;
    while (!pixelQueue.empty())
    {

        // Change function depending of the state of segmentation
        if (currentsegmentationsize > switchpercentage && switchparam != true)
        {
            switchparam = true;
        }

        std::shared_ptr<Pixl> currPixel = pixelQueue.front();
        pixelQueue.pop();

        visited.atChannel(currPixel->y, currPixel->x, 0) = currPixel->getId();

        // Precalculate BOundary
        int startX = std::max(currPixel->x - 1, 0);
        int endX = std::min(currPixel->x + 1, image.getCols() - 1);
        int startY = std::max(currPixel->y - 1, 0);
        int endY = std::min(currPixel->y + 1, image.getRows() - 1);

        // Add the seed pixel to the region
        for (int i = startX; i <= endX; ++i)
        {
            for (int j = startY; j <= endY; ++j)
            {
                // First visit of a pixel
                if ((visited.atChannel(j, i, 0)) == valctrl)
                {
                    std::shared_ptr<Pixl> neighbor(new Pixl(i, j, currPixel->getId()));
                    currentsegmentationsize++;
                    if (similarityFunction(image, currPixel, regions[currPixel->getId()], neighbor, threshold, switchparam))
                    {
                        pixelQueue.push(neighbor);
                        visited.atChannel(neighbor->y, neighbor->x, 0) = currPixel->getId();
                        regions[currPixel->getId()].addGerm(neighbor, image);
                    }
                }
                else
                {
                    if ((visited.atChannel(j, i, 0) != visited.atChannel(currPixel->y, currPixel->x, 0)))
                    {
                        uint32_t regionid = visited.atChannel(j, i, 0);
                        // Add as adjacent regions
                        regions[currPixel->getId()].addAdjacentRegion(&(regions[regionid]));
                        regions[regionid].addAdjacentRegion(&(regions[currPixel->getId()]));
                    }
                }
            }
        }
    }
}

// TODO compose Mat visited
//  TODO refrain from passing the whole matrice and just int
// TODO going through region or through a Mat of the visited state
// TODO fully template or keep as unsigned char
// Visited here currently does not represent anything but the original image
//  Create from the src image
Mat<uint8_t> composeSegMean(const Mat<uint8_t> &src, std::vector<Region> &regions);
// Create from the visited a representation of the segmentation in RGB

Mat<uint8_t> composeSegMeanColor(const Mat<uint8_t> &src, std::vector<Region> &regions);

// TODO 05/07/2024 Post GUI
// Unknog bud in SLic when this is outputted twice
Mat<unsigned char> composeSegRandCol(const Mat<uint8_t> &src, const std::vector<Region> &regions);
Mat<uint8_t> composeDisplayEdge(Mat<uint8_t> &visited, std::vector<Region> &regions);

#include <array>
// Test the best choice for Region Growing paramter
/*
void pipelineTest(std::string datapath)
{}
*/

// TODO choose to Store labspace or keep it as of right now
//  Pose des germes de façon régulière
//  K nombre de Superpixels
template <typename T>
std::vector<std::shared_ptr<Pixl>> selectSuperpixelCenter(const Mat<T> &img, std::vector<Region> &regions, int K)
{
    // Assuming we are in presence of a BGR image
    // Get a number of germ through the commandline
    std::vector<std::shared_ptr<Pixl>> superpixellist; // Stores the seeds
    int index = 0;

    // Approximation of the size of a superpixel

    int S = std::sqrt(img.getRows() * img.getCols() / K);
    // Push the first grid

    int height = img.getRows();
    int width = img.getCols();
    std::cout << "Sizze" << std::endl;

    std::cout << height << std::endl;
    std::cout << width << std::endl;

    for (int x = S / 2; x < width; x += S)
    {
        for (int y = S / 2; y < height; y += S)
        {
            // X y in the pixel
            // TODO temp solution
            std::shared_ptr<Pixl> pix(new Pixl(x, y, index));
            regions.push_back(Region());
            superpixellist.push_back(pix);

            regions[index].addGerm(pix, img);

            ++index;
        }
    }

    return superpixellist;
}

// TODO Should auto my way through this
//  Implemention sobel method
template <typename T>
float calcGradientSobel(int x, int y, const Mat<T> &image)
{
    // Check if the pixel is not at the border

    Mat<float> xchange = createMat3x3<float>(-1.0, 0.0, 1.0, -2.0, 0.0, 2.0, -1.0, 0.0, 1.0);

    Mat<float> ychange = createMat3x3<float>(-1.0, -2.0, -1.0, 0.0, 0.0, 0.0, 1.0, 2.0, 1.0);

    // Initialized at 0
    Mat<float> neighborhoodmat(3, 3, 3);
    // Zero the matrix just to be sure

    int startX = std::max(x - 1, 0);
    int endX = std::min(x + 1, image.getCols() - 1);
    int startY = std::max(y - 1, 0);
    int endY = std::min(y + 1, image.getRows() - 1);

    for (int i = startX; i <= endX; ++i)
    {
        for (int j = startY; j <= endY; ++j)
        {
            {
                neighborhoodmat.at(j - startY, i - startX)[0] = static_cast<float>(image.at(j, i)[0]);
                neighborhoodmat.at(j - startY, i - startX)[1] = static_cast<float>(image.at(j, i)[1]);
                neighborhoodmat.at(j - startY, i - startX)[2] = static_cast<float>(image.at(j, i)[2]);
            }
        }
    }

    // TODO

    Mat<float> neighborhoodmatc[3];
    neighborhoodmat.splitMat(neighborhoodmatc);

    // Test with squaring left
    //  COnsidering all element
    auto compute_component = [](const auto &channel, const auto &xchange, const auto &ychange)
    {
        float sum_x = (channel * xchange).sumComponents();
        float sum_y = (channel * ychange).sumComponents();

        // Return the squared gradient
        return sum_x * sum_x + sum_y * sum_y;
    };

    // Compute squared gradients for each channel (L, A, B)
    float gradientL = compute_component(neighborhoodmatc[0], xchange, ychange);
    float gradientA = compute_component(neighborhoodmatc[1], xchange, ychange);
    float gradientB = compute_component(neighborhoodmatc[2], xchange, ychange);

    float gradient = std::sqrt(gradientL) + std::sqrt(gradientA) + std::sqrt(gradientB);
    return gradient;
}

template <typename T>
inline double calcdistance5d(int x, int y, int i, int j, Mat<T> &image, const SlicParameter &param, int S)
{

    // DIstance of channel related values
    double distanceL = image.at(y, x)[0] - image.at(j, i)[0];
    double distanceA = image.at(y, x)[1] - image.at(j, i)[1];
    double distanceB = image.at(y, x)[2] - image.at(j, i)[2];
    double distanceLAB = std::sqrt(distanceL * distanceL + distanceA * distanceA + distanceB * distanceB);
    // DIstance on the plane
    // m= compactness
    //. When m is small, the resulting superpixels adhere more tightly to image boundaries, but have less regular size and shap
    //. When m is large, spatial proximity is more importantand the resulting superpixels are more compa
    // m is in the range 1 to 40
    // TODO, Optimization tend to change the result
    // TEst which is the best result
    double distanceXY = (i - x) * (i - x) + (j - y) * (j - y);
    double distanceXYnormalized = param.m_m * (std::sqrt(distanceXY) / S);

    double D = distanceLAB + distanceXYnormalized;

    return D;
}

// TODDO, optimize asit significantly slow thhe operations compared to opencv

// Move cluster centers to the lowest gradient position in a3 × 3 neighborhood.
template <typename T>
void moveCenterGradBased(const Mat<T> &image, std::vector<std::shared_ptr<Pixl>> supapixllist, const LabParam &param)

{

    Mat<T> imgLABspace(image);
    // Conversion to LABSPACEimg
    imgLABspace.toLabSpace(param);

    // Doubt the memory cost is worth it as callculation ere are done one time
    /*Mat<float> gradientCache(image.getRows(), image.getCols(),1);
    auto getGradient = [&](int x, int y)  {
        if (gradientCache.atChannel(y,x, 0) == 0.0) {
            gradientCache.atChannel(y,x, 0) = calcGradientSobel<T>(x, y, imgLABspace);
        }
        return gradientCache.atChannel(y,x,0);
    };*/

    // Calculate the duration

    auto starta = std::chrono::high_resolution_clock::now();

    for (std::shared_ptr<Pixl> &currentPixel : supapixllist)
    {
        // Create a cache for gradients

        float mingradient = calcGradientSobel<T>(currentPixel->x, currentPixel->y, imgLABspace);
        // float mingradient = getGradient(currentPixel->x, currentPixel->y);
        int startX = std::max(currentPixel->x - 1, 0);
        int endX = std::min(currentPixel->x + 1, image.getCols() - 1);
        int startY = std::max(currentPixel->y - 1, 0);
        int endY = std::min(currentPixel->y + 1, image.getRows() - 1);

        for (int i = startX; i <= endX; ++i)
        {

            for (int j = startY; j <= endY; ++j)
            {

                float currentgradient = calcGradientSobel<T>(i, j, imgLABspace);
                // float currentgradient = getGradient(i, j);
                if (mingradient > currentgradient)
                {
                    mingradient = currentgradient;
                    currentPixel->x = i;
                    currentPixel->y = j;
                }
            }
        }
    }

    auto enda = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> durationa = enda - starta;

    // Output the result in seconds
    std::cout << "Execution time In Move Center: " << durationa.count() << " seconds" << std::endl;
}

// TODO Either template it or something idk
/*
template <typename T>
bool CheckSpatialConnectivitySlic(const Mat<T> &image, float id, int x, int y, int Size)
{
    int startX = std::max(x - Size / 2, 0);
    int endX = std::min(x + Size / 2, image.getCols() - 1);
    int startY = std::max(y - Size / 2, 0);
    int endY = std::min(y + Size / 2, image.getRows() - 1);
    // For each pixel in a 2s*2s region around ck
    for (int i = startX; i <= endX; ++i)
    {
        for (int j = startY; j <= endY; ++j)
        {
            if (i == x && j == y)
            {
                continue;
            }
            if (image.at<T>(j, i)[1] == id)
            {
                return true;
            }
        }
    }
    return false;
}
*/

struct slicMapPair
{
    int first;
    int second;

    // Constructor for convenience
    slicMapPair() = default;
    slicMapPair(const int &fir, const int &secd) : first(fir), second(secd) {};
    slicMapPair(const slicMapPair &rhs) : first(rhs.first), second(rhs.second) {};

    // Overload equality operator for comparisons
    bool operator==(const slicMapPair &rhs) const
    {
        return first == rhs.first && second == rhs.second;
    }

    slicMapPair &operator=(const slicMapPair &rhs)
    {
        if (this != &rhs)
        {
            first = rhs.first;
            second = rhs.second;
        }
        return *this;
    }

    struct Hash
    {
        std::size_t operator()(const slicMapPair &p) const
        {
            return ((std::hash<int>()(p.first) << 1) ^ std::hash<int>()(p.second));
        }
    };
};
#include "omp.h"
// TODO Enforce spatial connectivity
// Replace std::map and pair ?
template <typename T>
void SLICAssignmentStep(const Mat<T> &image, std::vector<std::shared_ptr<Pixl>> &supapixllist, std::vector<Region> &regions, const SlicParameter &param)
{
    Mat<T> imgLABspace(image);
    // Conversion to LABSPACEimg
    imgLABspace.toLabSpace(param.m_labparam);
    int imgsize = image.getRows() * image.getCols();

    // typedef std::map<std::pair<int, int>, std::pair<int, int>> ClustersMap;
    // THe two method, unorderede map and map seem roughly equal
    typedef std::unordered_map<slicMapPair, slicMapPair, slicMapPair::Hash> ClustersMap;

    std::vector<ClustersMap> vectorOfMaps;
    int S = std::sqrt(imgsize / param.m_K);

    for (int i = 0; i < supapixllist.size(); ++i)
    {
        ClustersMap map;
        vectorOfMaps.push_back(map);
    }

    // Create a 2 channel matrix of the same size as the image to hold distance
    // Hold both distance and current region index respectively in first and second channel
    Mat<float> distance(image.getRows(), image.getCols(), 2);
    distance.setTo(std::numeric_limits<float>::infinity());

    int E = 0;
    while (E < 9)
    // Pas de convergence
    {
        std::cout << "First " << supapixllist.size() << std::endl;

        std::cout << "Execution time: A " << std::endl;

        // For each cluster center ck
#pragma omp parallel for

        for (std::shared_ptr<Pixl> &centerPixel : supapixllist)
        {
            // Precalculate BOundary
            int startX = std::max(centerPixel->x - 2 * S, 0);
            int endX = std::min(centerPixel->x + 2 * S, image.getCols() - 1);
            int startY = std::max(centerPixel->y - 2 * S, 0);
            int endY = std::min(centerPixel->y + 2 * S, image.getRows() - 1);

            for (int i = startX; i <= endX; ++i)
            {
                for (int j = startY; j <= endY; ++j)
                {
                    // Recompute the distance from current pixel and ck's cluster

                    float currentdis = calcdistance5d(centerPixel->x, centerPixel->y, i, j, imgLABspace, param, S);
                    if (distance.atChannel(j, i, 0) > currentdis)
                    {
#pragma omp critical

                        if (distance.atChannel(j, i, 0) == std::numeric_limits<float>::infinity())
                        {
                            distance.atChannel(j, i, 1) = centerPixel->getId();
                            // vectorOfMaps[centerPixel->getId()][std::make_pair(j, i)] = std::make_pair(j, i);
                            vectorOfMaps[centerPixel->getId()][slicMapPair(j, i)] = slicMapPair(j, i);
                        }

                        else if ((distance.atChannel(j, i, 0) != std::numeric_limits<float>::infinity()) &&
                                 (distance.atChannel(j, i, 1) != centerPixel->getId()))
                        {

                            // Exchange value between two cluster
                            int id = distance.atChannel(j, i, 1);
                            // Control the Connectivity betwwen the analysed cluster and the pixel

                            distance.atChannel(j, i, 1) = centerPixel->getId();
                            // vectorOfMaps[id].erase(std::make_pair(j, i));
                            vectorOfMaps[id].erase(slicMapPair(j, i));

                            // vectorOfMaps[centerPixel->getId()][std::make_pair(j, i)] = std::make_pair(j, i);
                            vectorOfMaps[centerPixel->getId()][slicMapPair(j, i)] = slicMapPair(j, i);
                        }
                        distance.atChannel(j, i, 0) = currentdis;
                    }
                }
            }
        }

        // Calculate the duration

        // Output the result in seconds

        for (std::shared_ptr<Pixl> &centerPixel : supapixllist)
        {

            int sum_x = 0;
            int sum_y = 0;
            int number = 1;
            // Recompute center
            // Item 5
            for (const auto &pair : vectorOfMaps[centerPixel->getId()])
            {

                sum_y += pair.second.first;
                sum_x += pair.second.second;
                number += 1;
            }
            int newy = (sum_y + centerPixel->y) / number;
            int newx = (sum_x + centerPixel->x) / number;
            centerPixel->x = newx;
            centerPixel->y = newy;
        }

        // Calculate the duration

        // Output the result in seconds

        E++;
    }

    int indexreg = 0;
    for (auto &map : vectorOfMaps)
    {
        for (auto &keyvalues : map)
        {

            std::shared_ptr<Pixl> neighbor(new Pixl(keyvalues.second.second, keyvalues.second.first, indexreg));
            regions[indexreg].addGerm(neighbor, image);
        }
        indexreg++;
    }
}

template <typename T>
void SlicAlgorithm(/*const char* datapath*/ std::vector<Region> &regions, const Mat<T> &image, const SlicParameter &param)
{
    // Clean the regions variable
    regions.clear();

    std::vector<std::shared_ptr<Pixl>> superPixelCenter = selectSuperpixelCenter(image, regions, param.m_K);

    std::chrono::milliseconds durationa;

    auto start = std::chrono::high_resolution_clock::now();
    moveCenterGradBased(image, superPixelCenter, param.m_labparam);

    SLICAssignmentStep(image, superPixelCenter, regions, param);
    auto stop = std::chrono::high_resolution_clock::now();
    durationa = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Duration of the entire operation was :" << durationa.count();
}

void testSlic();

#endif
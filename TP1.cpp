#include "Regions.hpp"
#include "./include/Imloader.h"


// TODO, Check which template are really useful

int moduloc(int dividend, int divisor)
{
    while (dividend >= divisor)
    {
        dividend -= divisor;
    }
    return dividend;
}


//TODO revoir
// Pose de germes
template <typename T>
std::shared_ptr<Pixl> putGerm(const Mat<T> &img, int x, int y, std::vector<Region> &regions)
{
    std::queue<std::shared_ptr<Pixl>> seedlist; // Stores the seeds
    // Add a new region
    regions.push_back(Region());

    std::shared_ptr<Pixl> pix(new Pixl(x, y, regions.size()));
    seedlist.push(pix);
    regions[pix->getIdInVec()].addGerm(pix, img);
    return pix;
}

// Pose des germes aléatoire
template <typename T>
std::queue<std::shared_ptr<Pixl>> putGerms(const Mat<T> &img, std::vector<Region> &regions, int germsquant)
{
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());

    std::uniform_int_distribution<int> distr(0, img.getRows() - 1);
    std::uniform_int_distribution<int> distar(0, img.getCols() - 1);

    std::queue<std::shared_ptr<Pixl>> seedlist; // Stores the seeds
    for (int i = 0; i < germsquant; i++)
    {
        regions.push_back(Region());
    }

    for (int i = 0; i < germsquant; i++)
    {
        int x = distr(generator); // generate a random number for x
        int y = distar(generator);
        std::shared_ptr<Pixl> pix(new Pixl(x, y, i));
        seedlist.push(pix);
        regions[i].addGerm(pix, img);
    }

    return seedlist;
}

// Pose des germes  heuristiques basé sur un histogramme

// Pose des germes aléatoire après division de l'image
template <typename T>
std::queue<std::shared_ptr<Pixl>> putGermsDivide(const Mat<T> &img, std::vector<Region> &regions, int n)
{
    std::srand(static_cast<unsigned int>(std::time(0)));

    // Get a number of germ through the commandline
    std::queue<std::shared_ptr<Pixl>> seedlist; // Stores the seeds
    int index = 0;
    int germquant = n * n;

    // Divide in n part on each side
    int partheight = img.getRows() / n;
    int partwidth = img.getCols() / n;
    // allocate as much region as there is seed

    if (partheight == 0 || partwidth == 0)
    { // partwidth  a des résultats intèressants
        partheight = 4;
        partwidth = 4;
        germquant = (img.getRows() * img.getCols()) / (partheight * partwidth);
    }

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
//TODO UPDATE, DOn't work currentyl
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
    if (img.channels() == 3 && treat == 3)
    {
        // Gaussian Blur
        kernel = createMatNxN(3, 1.0 / 16, 2.0 / 16, 1.0 / 16, 2.0 / 16, 4.0 / 16, 2.0 / 16, 1.0 / 16, 2.0 / 16, 1.0 / 16);
        convolved = applyFilterConv3C3(img, kernel);
        return convolved;
    }
    else if (img.channels() == 3 && treat == 2)
    {
        medianBlurCustom(img, convolved);
        return convolved;
    }
    else if (img.channels() == 1 && treat == 1)
    {
        // Mean blur
        kernel = createMatNxN(3, 1.0 / 9, 1.0 / 9, 1.0 / 9, 1.0 / 9, 1.0 / 9, 1.0 / 9, 1.0 / 9, 1.0 / 9, 1.0 / 9);
        convolved = applyFilterConv3(img, kernel);
        return convolved;
    }

    return img;
}
*/
// TODO Consider the row major order
// TODO Visited might as well should be initalized here
// Execute eRegionGrowing algorithm with 1c SImilarity function
template <typename T>
void regionGrowing1c(const Mat<T> &image, std::queue<std::shared_ptr<Pixl>> &pixelQueue,
                     const float threshold, Mat<uint32_t> &visited, std::vector<Region> &regions)
{
    while (!pixelQueue.empty())
    {
        std::shared_ptr<Pixl> currPixel = pixelQueue.front();
        pixelQueue.pop();
        visited.atChannel(currPixel->y, currPixel->x, 0) = currPixel->getIdInImg();
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
                if ((visited.atChannel(j, i, 0)) == 0)
                {
                    std::shared_ptr<Pixl> neighbor(new Pixl(i, j, currPixel->getIdInVec()));
                    if (isSimilarIntensity1c(image, currPixel, (neighbor), threshold))
                    {
                        pixelQueue.push(neighbor);
                        // Add the neighboring pixel to the region
                        visited.atChannel(neighbor->y, neighbor->x, 0) = currPixel->getIdInImg();
                        regions[currPixel->getIdInVec()].addGerm(neighbor, image);
                    }
                }
                // Pixel already visited, we decide if we want to add him as neighbor
                else
                {
                    // TODO Remnant from in Region growing fusion ?
                    // regions[static_cast<int>(visited.at<int>(i, j)) - 1].getSizePix() > 0)
                    if ((visited.atChannel(j, i, 0) != visited.atChannel(currPixel->y, currPixel->x, 0)))
                    {
                        unsigned int regionid = visited.atChannel(j, i, 0) - 1;
                        // Add as adjacent regions
                        regions[currPixel->getIdInVec()].addAdjacentRegion(&(regions[regionid]));
                        regions[regionid].addAdjacentRegion(&(regions[currPixel->getIdInVec()]));
                    }
                }
            }
        }
    }
}

// TODO Consider the row major order
// TODO Visited might as well should be initalized here
// Execute eRegionGrowing algorithm with 3c SImilarity function
template <typename T>
void regionGrowing3c(const Mat<T> &image, std::queue<std::shared_ptr<Pixl>> &pixelQueue,
                     const float threshold, Mat<uint32_t> &visited, std::vector<Region> &regions)
{
    while (!pixelQueue.empty())
    {
        std::shared_ptr<Pixl> currPixel = pixelQueue.front();
        pixelQueue.pop();
        visited.atChannel(currPixel->y, currPixel->x, 0) = currPixel->getIdInImg();
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
                if ((visited.atChannel(j, i, 0)) == 0)
                {
                    std::shared_ptr<Pixl> neighbor(new Pixl(i, j, currPixel->getIdInVec()));
                    if (
                        isSimilarIntensity3c(image, currPixel, (neighbor), threshold)
                        // isSimidlarLuminance3cGlobal(image, regions[currPixel->getIdInVec()], (neighbor), threshold)
                        // isSimilarIntensity3cGlobal(image, regions[currPixel->getIdInVec()], (neighbor), threshold)
                    )
                    {
                        // Add the neighboring pixel to the region
                        pixelQueue.push(neighbor);
                        // Add the neighboring pixel to the region
                        visited.atChannel(neighbor->y, neighbor->x, 0) = currPixel->getIdInImg();
                        regions[currPixel->getIdInVec()].addGerm(neighbor, image);
                    }
                }
                else
                {
                    // TODO Remnant from in Region growing fusion ?
                    // in if : && regions[static_cast<int>(visited.at<int>(i, j)) - 1].getSizePix() > 0)

                    if ((visited.atChannel(j, i, 0) != visited.atChannel(currPixel->y, currPixel->x, 0)))
                    {
                        uint32_t regionid = visited.atChannel(j, i, 0) - 1;
                        // Add as adjacent regions
                        regions[currPixel->getIdInVec()].addAdjacentRegion(&(regions[regionid]));
                        regions[regionid].addAdjacentRegion(&(regions[currPixel->getIdInVec()]));
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
            int endY = std::min(j+ 1, image.getRows() - 1);

            for (int x = startX; x <= endX; ++x)
            {
                for (int y = startY; y <= endY; ++y)
                { // TODO,
                    if ((used.atChannel(y, x, 0) > 0) && (used.atChannel(y, x, 0) != used.atChannel(j, i, 0)) &&
                        isSimilarMeanAverage(regions[used.atChannel(y, x, 0) - 1], regions[used.atChannel(j, i, 0) - 1], threshold))
                    {
                        int regionid = used.atChannel(y, x, 0) - 1;
                        // Add as adjacent regions
                        regions[used.atChannel(y, x, 0) - 1].addAdjacentRegion(&(regions[used.atChannel(j, i, 0) - 1]));
                        regions[used.atChannel(j, i, 0) - 1].addAdjacentRegion(&(regions[used.atChannel(y, x, 0) - 1]));
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
    // return isSimidlarLuminance3cGlobal(image, regions[currentPixel->getIdInVec()], neighbor, threshold);
};

template <typename T>
void adaptiveRegionGrowing(const Mat<T> &image, std::queue<std::shared_ptr<Pixl>> &pixelQueue,
                           const float threshold, Mat<unsigned int> &visited, std::vector<Region> &regions, int adaptative)
{

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

        visited.atChannel(currPixel->y, currPixel->x, 0) = currPixel->getIdInImg();

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
                if ((visited.atChannel(j, i, 0)) == 0)
                {
                    std::shared_ptr<Pixl> neighbor(new Pixl(i, j, currPixel->getIdInVec()));
                    currentsegmentationsize++;
                    if (similarityFunction(image, currPixel, regions[currPixel->getIdInVec()], neighbor, threshold, switchparam))
                    {
                        pixelQueue.push(neighbor);
                        visited.atChannel(neighbor->y, neighbor->x, 0) = currPixel->getIdInImg();
                        regions[currPixel->getIdInVec()].addGerm(neighbor, image);
                    }
                }
                else
                {
                    if ((visited.atChannel(j, i, 0) != visited.atChannel(currPixel->y, currPixel->x, 0)))
                    {
                        uint32_t regionid = visited.atChannel(j, i, 0) - 1;
                        // Add as adjacent regions
                        regions[currPixel->getIdInVec()].addAdjacentRegion(&(regions[regionid]));
                        regions[regionid].addAdjacentRegion(&(regions[currPixel->getIdInVec()]));
                    }
                }
            }
        }
    }
}

// Create from the visited a representation of the segmentation in grayscale
Mat<uint8_t> composeSegMean(const Mat<uint32_t> &visited, std::vector<Region> &regions)
{
    Mat<uint8_t> resultMat(visited.getRows(), visited.getCols(), visited.getChannels(),
                           visited.getClrspace());

    for (Region &region : regions)
    {
        if (region.getSizePix() > 0)
        {
            for (std::shared_ptr<Pixl> &pixelptr : region.pixels)
            {
                resultMat.atChannel(pixelptr->y, pixelptr->x, 0) = static_cast<uint8_t>(region.getMeanIntensity());
            }
        }
    }
    return resultMat;
}

// Create from the visited a representation of the segmentation in RGB

Mat<uint8_t> composeSegMeanColor(const Mat<uint32_t> &visited, std::vector<Region> &regions)
{
    Mat<uint8_t> resultMat(visited.getRows(), visited.getCols(), 3);
    for (Region &region : regions)
    {
        if (region.getSizePix() > 0)
        {
            Vec<uint32_t> regmeanclr = region.computeColorIntensity(visited);
            
            for (std::shared_ptr<Pixl> &pixelptr : region.pixels)
            {
                resultMat.at(pixelptr->y, pixelptr->x)[0] = regmeanclr.x;
                resultMat.at(pixelptr->y, pixelptr->x)[1] = regmeanclr.y;
                resultMat.at(pixelptr->y, pixelptr->x)[2] = regmeanclr.z;
            }
        }
    }
    return resultMat;
}

Mat<uint8_t> composeSegRandCol(const Mat<uint32_t> &visited, std::vector<Region> &regions)
{

    // Seed the random number generator
    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<uint8_t> colorR;
    std::vector<uint8_t> colorB;
    std::vector<uint8_t> colorG;

    // Step between regions colour
    for (int i = 1; i < 255; ++i)
    {
        colorB.push_back(i);
        colorR.push_back(i);
        colorG.push_back(i);
    }
    // using built-in random generator:
    std::shuffle(colorB.begin(), colorB.end(), gen);
    std::shuffle(colorG.begin(), colorG.end(), gen);
    std::shuffle(colorR.begin(), colorR.end(), gen);

    Mat<uint8_t> resultMat(visited.getRows(), visited.getCols(), 3);
    for (Region &region : regions)
    {

        if (region.getSizePix() > 0)
        {
            int id = region.pixels[0]->getIdInImg() % 255;
            for (std::shared_ptr<Pixl> &pixelptr : region.pixels)
            {
                resultMat.at(pixelptr->y, pixelptr->x)[0] = colorB[id];
                resultMat.at(pixelptr->y, pixelptr->x)[1] = colorB[id];
                resultMat.at(pixelptr->y, pixelptr->x)[2] = colorB[id];
            }
        }
    }
    return resultMat;
}

Mat<uint8_t> composeDisplayEdge(Mat<uint32_t> &visited, std::vector<Region> &regions)
{

    // Idée, deux parcours complet de l'image.... ou stocker les informations sur la couleurs dans région aussi
    Mat<uint8_t> intermediaryMat = composeSegMean(visited, regions);

    Mat<uint8_t> kernel = createMat3x3<uint8_t>(3.0f, -1.0f, 0.0f,
                                                1.0f, -2.0f, 0.0f,
                                                2.0f, -1.0f, 0.0f);
    Mat<uint8_t> resultMat = applyFilterConv3<uint8_t>(intermediaryMat, kernel);

    return resultMat;
}

/*
int main(int argc, char ** argv) {
    int i;

    // Paramètres
    int nbthreads = 1;
    int seedquant = 100;
    float threshold = 4;
    // lecture des arguments

   std::string filepath = "new.png";
   // std::string filepath = "./img.jpg";

    // std::string filepath = "./img-1z.png";

    cv::Mat img = imgTreat(filepath, 3);
    cv::Mat imgb = imgTreat(filepath, 3);


    std::vector < Region > regions;
    std::vector < Region > regionsb;


        std::queue < std::shared_ptr < Pixl >> seedlist = putGermsDivide(img, regions, 15);
    std::queue < std::shared_ptr < Pixl >> seedlistb = putGermsDivide(imgb, regionsb, 15);


    cv::Mat visited;
    cv::Mat visitedb;
    std::chrono::milliseconds durationa;
    std::chrono::milliseconds durationb;
    std::cout << img.channels() << "canaux " << std::endl;
    if (img.channels() == 3) {

        std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
        visited = cv::Mat::zeros(img.getRows(), img.getCols(), CV_32SC3);

        std::cout << "growing A " << std::endl;

        regionGrowing3c(img, seedlist, threshold, visited, regions);
        std::cout << "growing A done" << std::endl;
        fuseRegions(regions, threshold);
         fuseRegions(regions, threshold );


        std::chrono::high_resolution_clock::time_point stop = std::chrono::high_resolution_clock::now();
        durationa = std::chrono::duration_cast < std::chrono::milliseconds > (stop - start);

        start = std::chrono::high_resolution_clock::now();
        visitedb = cv::Mat::zeros(img.getRows(), img.getCols(), CV_32SC3);
        // Sharpened
        regionGrowing3c(imgb, seedlistb, threshold, visitedb, regionsb);


        fuseRegions(regionsb, threshold);
        fuseRegions(regionsb, threshold);

        stop = std::chrono::high_resolution_clock::now();

        durationb = std::chrono::duration_cast < std::chrono::milliseconds > (stop - start);

    } else if (img.channels() == 1) {

        visited = cv::Mat::zeros(img.getRows(), img.getCols(), CV_8UC3);
        regionGrowing1c(img, seedlist, threshold, visited, regions);
    }

    int taillerega = 0;
    for (int i = 0; i < regions.size(); i++) {
        //std::cout << regions[i].getSizePix() << "regions a with is first being " <<std::endl;

        // std::cout<<regions[i].pixels[0] -> getIdInVec() << regions[i].getMeanIntensity()<< std::endl;
        taillerega += regions[i].getSizePix();
    }

    int tailleregb = 0;
    for (int i = 0; i < regionsb.size(); i++) {
        //    std::cout << regionsb[i].getSizePix() << "regions b" << std::endl;
        tailleregb += regionsb[i].getSizePix();
    }
    float percentSegmenteda = taillerega * 100 / (img.getRows() * img.getCols());
    float percentSegmentedb = tailleregb * 100 / (imgb.getRows() * imgb.getCols());

    std::cout << percentSegmenteda << " de l'image à été segementé en " << durationa.count() << " millisecondes" << std::endl;
    std::cout << percentSegmentedb << " de la version de l'image à été segementé en " << durationb.count() << " millisecondes" << std::endl;

    // 1 tableau de thread
    std::thread tabt[nbthreads];

    cv::Mat display = composeSegMean(visited, regions);
    cv::Mat displayRandCol = composeSegRandCol(visited, regions);

    cv::Mat displayb = composeSegMean(visitedb, regionsb);
    cv::Mat displayRandColb = composeSegRandCol(visitedb, regionsb);

    cv::Mat composeDisplayEdgea = composeDisplayEdge(visited, regions);
    cv::Mat composeDisplayEdgeb = composeDisplayEdge(visitedb, regionsb);


    cv::imshow("ImageB originelle", imgb);
    cv::imshow("Imagea originelle Traitée", img);

    cv::imshow("ImageA Traitée segmentée meangraylevel", display);
    cv::imshow("ImageA Traitée segmentée rand colour", displayRandCol);



    cv::imshow("ImageB  segmentée meangraylevel", displayb);
    cv::imshow("ImageB  segmentée rand colour", displayRandColb);



    cv::waitKey(0);

 }

*/

#include <array>
// Test the best choice for Region Growing paramter
/*
void pipelineTest(std::string datapath)
{
// Seed number equal image division squared
// Threshold
    // Tested Parameters
    std::array<int, 4> imgdivide{{5, 10, 100, 1000}};
    std::array<int, 5> thresholdarr{{1, 4, 9, 15, 30}};
    std::array<int, 3> treatmentarray{{0, 2, 3}};
    std::array<int, 3> adaptativenumarrray{{0, 50, 100}};

    int resultsholdersize = imgdivide.size() * treatmentarray.size() * thresholdarr.size();
    std::vector<int> imgdivideindex;
    std::vector<int> treatmentindex;
    std::vector<int> thresholdindex;
    std::vector<int> adaptnum;
    std::vector<std::chrono::milliseconds> timeresult;
    std::vector<int> percentageresult;
    std::vector<float> scoreresult;
    int az = 0;
    //
    for (int i : treatmentarray)
    {
        cv::Mat img = imgTreat(datapath, i);
        for (int j : imgdivide)
        {
            for (int k : thresholdarr)
            {
                for (int l : adaptativenumarrray)
                {
                    std::vector<Region> regions;
                    std::queue<std::shared_ptr<Pixl>> seedlist = putGermsDivide(img, regions, j);

                    az++;
                    cv::Mat visited;
                    std::chrono::milliseconds duration;

                    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
                    visited = cv::Mat::zeros(img.getRows(), img.getCols(), CV_32SC3);
                    adaptiveRegionGrowing(img, seedlist, k, visited, regions, l);
                    fuseRegions(regions, k);
                    fuseRegions(regions, k * 4);
                    fuseRegions(regions, k);

                    std::chrono::high_resolution_clock::time_point stop = std::chrono::high_resolution_clock::now();
                    duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

                    std::cout << "Reelmex" << az << std::endl;

                    //
                    int taillereg = 0;
                    for (int i = 0; i < regions.size(); i++)
                    {
                        //    std::cout << regionsb[i].getSizePix() << "regions b" << std::endl;
                        taillereg += regions[i].getSizePix();
                    }
                    //  std::cout<<"Element" << az <<std::endl;

                    float percentSegmented = taillereg * 100 / (img.getRows() * img.getCols());

                    std::cout << percentSegmented << std::endl;

                    // Adding metrics and sotring parameter for this iteration

                    if (percentSegmented > 70)
                    {
                        std::cout << "write";
                        imgdivideindex.push_back(j);
                        treatmentindex.push_back(i);
                        thresholdindex.push_back(k);
                        adaptnum.push_back(l);
                        timeresult.push_back(duration);
                        percentageresult.push_back(percentSegmented);

                        // FInal segmentation output
                        cv::Mat segmentedmean = composeSegMean(visited, regions);
                        cv::Mat segmentedrandcol = composeSegRandCol(visited, regions);
                        // cv::Mat f = composeSegMeanColor(visited, regions); weird element inside

                        std::string combinedString;
                        combinedString += datapath;
                        combinedString += " Treat:" + std::to_string(i);
                        combinedString += " Divs:" + std::to_string(j);
                        combinedString += " Tol:" + std::to_string(k);
                        combinedString += " Adapt:" + std::to_string(l);
                        combinedString += " Timems:" + std::to_string(duration.count());
                        combinedString += " %:" + std::to_string(percentSegmented);
                        std::string mean = " Mean:";
                        std::string randcol = " RandCOl:";

                        cv::imwrite(combinedString + mean + ".jpg", segmentedmean);
                        cv::imwrite(combinedString + randcol + ".jpg", segmentedrandcol);
                    }

                    // scoring(img, visited);
                    // scoreresult.push_back(percentSegmented);
                }
            }
        }
        // Trate
        //    cv::imwrite( std::to_string(i) + ".jpg"  , img);
    }

    std::cout << "FInished" << az;
}
*/
/*
void scoring(const cv::Mat &image, std::queue<std::shared_ptr<Pixl>> &pixelQueue,
             const float threshold, cv::Mat &visited)
{
}
*/

// TODO choose to Store labspace or keep it as of right now
//  Pose des germes de façon régulière
//  K nombre de Superpixels
template <typename T>
std::vector<std::shared_ptr<Pixl>> selectSuperpixelCenter(const Mat<T> &img, std::vector<Region> &regions, int K)
{
    // Assuming we are in presence of a BGR image
    // Mat<T> imgLBspace(img);
    // img.toLbSpace();

    // Get a number of germ through the commandline
    std::vector<std::shared_ptr<Pixl>> superpixellist; // Stores the seeds
    int index = 0;

    // Approximation of the size of a superpixel
    int S = sqrt(img.getRows() * img.getCols() / K);
    // Push the first grid
    int x = S / 2;
    int y = S / 2;

    int height = img.getRows();
    int width = img.getCols();
    // allocate as much region as there is seed
    // for (int i = 0; i < germquant; ++i) {
    //   regions.push_back(Region());
    //}

    while (x < height)
    {
        while (y < width)
        {
            // X y in the pixel
            std::shared_ptr<Pixl> pix(new Pixl(y, x, index));
            regions.push_back(Region());
            superpixellist.push_back(pix);

            regions[index].addGerm(pix, img);
            ++index;
            y += S;
        }
        y = S / 2;
        x += S;
    }
    return superpixellist;
}

// Implemention sobel method
template <typename T>
double calcGradientSobel(int x, int y, const Mat<T> &image)
{
    // Check if the pixel is not at the border
    Mat<T> xchange = createMat3x3<T>(3, -1.0, 0.0, 1.0, -2.0, 0.0, 2.0, -1.0, 0.0, 1.0);

    Mat<T> ychange = createMat3x3<T>(3, -1.0, -2.0, -1.0, 0.0, 0.0, 0.0, 1.0, 2.0, 1.0);

    // Create a 3x3 region  around the specified pixel
    /*cv::Rect neighbourhoodrec(std::max(0, x - 1), std::max(0, y - 1), 3, 3);
    // Cut off part not in the neighbourhood if there is


          std::cout<<neighbourhoodrec.y<<std::endl;
    neighbourhoodrec.width = std::min(neighbourhoodrec.width, image.getCols() - neighbourhoodrec.x);
    neighbourhoodrec.height = std::min(neighbourhoodrec.height, image.getRows() - neighbourhoodrec.y);*/

    int halfsize = 3 / 2;
    Mat<float> neighborhoodmat(3, 3, 3);
    // TODO, change method
    for (int i = -halfsize; i <= halfsize; ++i)
    {
        for (int j = -halfsize; j <= halfsize; ++j)
        {
            int newX = x + j;
            int newY = y + i;

            // Check if the pixel is within the image boundaries
            if (newX >= 0 && newX < image.getCols() && newY >= 0 && newY < image.getRows())
            {
                neighborhoodmat.at(j + halfsize, i + halfsize)[0] = image.at(j + halfsize, i + halfsize)[0];
                neighborhoodmat.at(j + halfsize, i + halfsize)[1] = image.at(j + halfsize, i + halfsize)[1];
                neighborhoodmat.at(j + halfsize, i + halfsize)[2] = image.at(j + halfsize, i + halfsize)[2];
            }
            // If outside image boundaries, pad with zeros
        }
    }

    // TODO Contorl
    Mat<float> neighborhoodmatc[3];
    neighborhoodmat.splitMat(neighborhoodmatc);

    // COnsidering all element

    double gradientL = std::sqrt(std::pow(((neighborhoodmatc[0] * xchange).sumComponents()), 2.0) +
                                 std::pow(((neighborhoodmatc[0] * ychange).sumComponents()), 2.0));

    double gradientA = std::sqrt(std::pow(((neighborhoodmatc[1] * xchange).sumComponents()), 2.0) +
                                 std::pow(((neighborhoodmatc[1] * ychange).sumComponents()), 2.0));

    double gradientB = std::sqrt(std::pow(((neighborhoodmatc[2] * xchange).sumComponents()), 2.0) +
                                 std::pow(((neighborhoodmatc[2] * ychange).sumComponents()), 2.0));

    // Compare with direct loop on the mat
    /*
     double gradientL = sqrt(pow((cv::sum(neighborhoodmatc[0] * xchange))[0], 2.0) + pow((cv::sum(neighborhoodmatc[0] * ychange))[0], 2.0));

    double gradientA = sqrt(pow((cv::sum(neighborhoodmatc[1] * xchange))[0], 2.0) + pow((cv::sum(neighborhoodmatc[1] * ychange))[0], 2.0));

    double gradientB = sqrt(pow((cv::sum(neighborhoodmatc[2] * xchange))[0], 2.0) + pow((cv::sum(neighborhoodmatc[2] * ychange))[0], 2.0));


    */

    double gradient = gradientL + gradientA + gradientB;

    return gradient;
}

template <typename T>
double calcdistance5d(int x, int y, int i, int j, Mat<T> &image, int S)
{

    // DIstance of channel related values
    double distanceL = image.at(y, x)[0] - image.at(j, i)[0];
    double distanceA = image.at(y, x)[1] - image.at(j, i)[1];
    double distanceB = image.at(y, x)[2] - image.at(j, i)[2];
    double distanceLAB = sqrt((pow(distanceL, 2.0) + pow(distanceA, 2.0) + pow(distanceB, 2.0)));
    // DIstance on the plane
    // m= compactness
    //. When m is small, the resulting superpixels adhere more tightly to image boundaries, but have less regular size and shap
    //. When m is large, spatial proximity is more importantand the resulting superpixels are more compa
    // m is in the range 1 to 40
    int m = 15;
    double distanceXY = (std::pow((i - x), 2.0) + std::pow((j - y), 2.0));
    double distanceXYnormalized = m * (std::sqrt(distanceXY) / S);

    double D = distanceLAB + distanceXYnormalized;
    return D;
}

// Move cluster centers to the lowest gradient position in a3 × 3 neighborhood.
template <typename T>
void moveCenterGradBased(const Mat<T> &image, std::vector<std::shared_ptr<Pixl>> supapixllist)
{
    Mat<T> imgLABspace(image);
    // Conversion to LABSPACEimg
    imgLABspace.toLabSpace();

    for (std::shared_ptr<Pixl> &currentPixel : supapixllist)
    {
        double mingradient = calcGradientSobel<T>(currentPixel->x, currentPixel->y, imgLABspace);
        int startX = std::max(currentPixel->x - 1, 0);
        int endX = std::min(currentPixel->x + 1, image.getCols() - 1);
        int startY = std::max(currentPixel->y - 1, 0);
        int endY = std::min(currentPixel->y + 1, image.getRows() - 1);

        for (int i = startX; i <= endX; ++i)
        {

            for (int j = startY; j <= endY; ++j)
            {

                double currentgradient = calcGradientSobel<T>(i, j, imgLABspace);
                if (mingradient > currentgradient)
                {
                    mingradient = currentgradient;
                    currentPixel->x = i;
                    currentPixel->y = j;
                }
            }
        }
    }
}

/* TODO What was it for ? Just a test ?
 void recomputeCenter( cv::Mat& image, std::vector<std::shared_ptr<Pixl>> &supapixllist, const auto& vectorOfMaps ){
            for (std::shared_ptr<Pixl> &centerPixel : supapixllist)
        {

        int sum_x = 0;
        int sum_y = 0;
        int number = 1;
              double distanceL = 0;
            double distanceA = 0;
            double distanceB = 0;
        // Recompute centerk

         // std::cout<<vectorOfMaps[1].size()<<std::endl;

            for (const auto &[key, position] : vectorOfMaps[centerPixel->getIdInVec()])
            {
                sum_y += position.first;
                sum_x += position.second;
            distanceL += image.at<cv::Vec3b>(position.first, position.second)[0];
             distanceA += image.at<cv::Vec3b>(position.first, position.second)[1] ;
             distanceB += image.at<cv::Vec3b>(position.first, position.second)[2];
                number += 1;
            }
            int newy = (sum_y + centerPixel->y) / (number );
            int newx = (sum_x + centerPixel->x) / (number );
             double newdistanceL = (distanceL + distanceL+image.at<cv::Vec3b>(centerPixel->y, centerPixel->x)[0])/number;
              double newdistanceA = (distanceA + distanceA+image.at<cv::Vec3b>(centerPixel->y, centerPixel->x)[1])/number;
              double newdistanceB =(distanceB + distanceB+image.at<cv::Vec3b>(centerPixel->y, centerPixel->x)[2])/number;
            centerPixel->x = newx;
            centerPixel->y = newy;
            image.at<cv::Vec3b>(centerPixel->y, centerPixel->x)[0] = newdistanceL;
            image.at<cv::Vec3b>(centerPixel->y, centerPixel->x)[1] = newdistanceA;
            image.at<cv::Vec3b>(centerPixel->y, centerPixel->x)[2] = newdistanceB;

            // Output the intermediate results
            std::cout << "Sum of y coordinates: " << sum_y << std::endl;
            std::cout << "Sum of x coordinates: " << sum_x << std::endl;
            std::cout << "Total number of points: " << number << std::endl;
            std::cout << "Old y coordinate: " << newy << std::endl;
            std::cout << "Old x coordinate: " << centerPixel->x << std::endl;
            std::cout << "New y coordinate: " << centerPixel->y <<std::endl;
            std::cout << "New x coordinate: " << newx << std::endl;
               std::cout << std::endl;

        }
     }    */

// cols == x rows == y
#include <typeinfo>

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

//TODO What was it about again
template <typename T>
void SLICAssignmentStep(const Mat<T> &image, std::vector<std::shared_ptr<Pixl>> &supapixllist, std::vector<Region> &regions, int K)
{
    Mat<T> imgLABspace(image);
    // Conversion to LABSPACEimg
    imgLABspace.toLabSpace();
    int imgsize = image.getRows() * image.getCols();

    typedef std::map<std::pair<int, int>, std::pair<int, int>> ClustersMap;
    std::vector<ClustersMap> vectorOfMaps;
    int S = sqrt(imgsize / K);

    for (int i = 0; i < supapixllist.size(); ++i)
    {

        ClustersMap map;
        vectorOfMaps.push_back(map);
    }

    // Create a 2 channel matrix of the same size as the image to hold distance
    // Hold both distance and current region index respectively in first and second channel
    Mat<float>distance(image.getRows(), image.getCols(),2);
    distance.setTo(std::numeric_limits<float>::infinity());
    int E = 0;
    while (E < 10)
    // Pas de convergence
    {
        std::cout << "First " << supapixllist.size() << std::endl;

        // For each cluster center ck
        for (std::shared_ptr<Pixl> &centerPixel : supapixllist)
        {
            // Precalculate BOundary
            int startX = std::max(centerPixel->x - 2 * S, 0);
            int endX = std::min(centerPixel->x + 2 * S, image.getCols() - 1);
            int startY = std::max(centerPixel->y - 2 * S, 0);
            int endY = std::min(centerPixel->y + 2 * S, image.getRows() - 1);

            // Enforce spatial connecitvity currentyl don't work
            // TODO ENforce it here after buffering the usable
            //cv::Range rowRange(startY, endY);
            //cv::Range colRange(startX, endX);

            // For each pixel in a 2s*2s region around ck
            for (int i = startX; i <= endX; ++i)
            { // TODO <= necessary ?
                for (int j = startY; j <= endY; ++j)
                {
                    // Recompute the distance from current pixel and ck's cluster
                    double currentdis = calcdistance5d(centerPixel->x, centerPixel->y, i, j, imgLABspace, S);
                    if (distance.atChannel(j,i,0) > currentdis)
                    {
                        if (distance.atChannel(j,i,1) == centerPixel->getIdInVec())
                        {
                            // Already in the correct place we simply update the distance late
                            // May not even happend once ?...
                            // TODO Verify the usefulness of this
                        }

                        else if (distance.atChannel(j,i,0) == std::numeric_limits<double>::infinity())
                        {
                            distance.atChannel(j,i,1) = centerPixel->getIdInVec();
                            vectorOfMaps[centerPixel->getIdInVec()][std::make_pair(j, i)] = std::make_pair(j, i);
                        }

                        else if ((distance.atChannel(j,i,0) != std::numeric_limits<double>::infinity()) && 
                                (distance.atChannel(j,i,1) != centerPixel->getIdInVec()))
                        {

                            // Exchange value between two cluster
                            int id = distance.atChannel(j,i,1);
                            // Control the Connectivity betwwen the analysed cluster and the pixel

                            distance.atChannel(j,i,1) = centerPixel->getIdInVec();
                            vectorOfMaps[id].erase(std::make_pair(j, i));
                            vectorOfMaps[centerPixel->getIdInVec()][std::make_pair(j, i)] = std::make_pair(j, i);
                        }
                        distance.atChannel(j,i,0) = currentdis;
                    }
                }
            }
        }

        for (std::shared_ptr<Pixl> &centerPixel : supapixllist)
        {

            int sum_x = 0;
            int sum_y = 0;
            int number = 1;
            // Recompute center
            for (const auto &pair : vectorOfMaps[centerPixel->getIdInVec()])
            {

                sum_y += pair.second.first;
                sum_x += pair.second.second;
                number += 1;
            }
            int newy = (sum_y + centerPixel->y) / number;
            int newx = (sum_x + centerPixel->x) / number;
            centerPixel->x = newx;
            centerPixel->y = newy;
            /*

                       std::cout << "Sum of y coordinates: " << sum_y << std::endl;
                       std::cout << "Sum of x coordinates: " << sum_x << std::endl;
                       std::cout << "Total number of points: " << number << std::endl;
                       std::cout << "Old y coordinate: " << newy << std::endl;
                       std::cout << "Old x coordinate: " << centerPixel->x << std::endl;
                       std::cout << "New y coordinate: " << centerPixel->y <<std::endl;
                       std::cout << "New x coordinate: " << newx << std::endl;*/
        }

        // No indication about the calculation of the residual error so we take 10 since it usually converge by 10

        E++;
    }
    std::cout << "ze";
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

template<typename T>
void SlicAlgorithm(const char* datapath, int K)
{

    Mat<T> image = loadImg(datapath, true);

    /*
    std::vector<Region> regions;
    std::vector<std::shared_ptr<Pixl>> superPixelCenter = selectSuperpixelCenter(image, regions, K);

    std::chrono::milliseconds durationa;

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    moveCenterGradBased(image, superPixelCenter);

    SLICAssignmentStep(image, superPixelCenter, regions, K);

    std::chrono::high_resolution_clock::time_point stop = std::chrono::high_resolution_clock::now();
    durationa = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    std::cout << "duration A is :" << durationa.count();
    Mat<T> displayRandCol = composeSegRandCol(image, regions);
    Mat<T> display = composeSegMeanColor(image, regions);
   
    */
}

int main(int argc, char **argv)
{
    SlicAlgorithm<unsigned char>("0.jpg", 100);
    // pipeline(data);
}

// Regionns wide modiication

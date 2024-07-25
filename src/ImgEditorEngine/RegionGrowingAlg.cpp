#include "RegionGrowingAlg.h"

// TODO, Check which template are really useful

int moduloc(int dividend, int divisor)
{
    while (dividend >= divisor)
    {
        dividend -= divisor;
    }
    return dividend;
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



// TODO compose Mat visited
//  TODO refrain from passing the whole matrice and just int
// TODO going through region or through a Mat of the visited state
// TODO fully template or keep as unsigned char
// Visited here currently does not represent anything but the original image
//  Create from the src image
Mat<uint8_t> composeSegMean(const Mat<uint8_t> &src, std::vector<Region> &regions)
{
    Mat<uint8_t> resultMat(src.getRows(), src.getCols(), src.getChannels(),
                           src.getClrspace());

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

Mat<uint8_t> composeSegMeanColor(const Mat<uint8_t> &src, std::vector<Region> &regions)
{
    Mat<uint8_t> resultMat(src.getRows(), src.getCols(), 3);
    for (Region &region : regions)
    {
        if (region.getSizePix() > 0)
        {
            Vec<uint8_t> regmeanclr = (region.computeColorIntensity(src)).convertDt<uint8_t>();

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
//TODO REmove the write here
// TODO 05/07/2024 Post GUI
// Unknog bud in SLic when this is outputted twice
Mat<uint8_t> composeSegRandCol(const Mat<uint8_t> &src, const std::vector<Region> &regions)
{
    Mat<uint8_t> resultMat(src.getRows(), src.getCols(), 3, PixlColorSpace::RGB);
    std::cout<<std::endl;
   
    std::cout<<src.getRows()<<std::endl;
    std::cout<<src.getCols()<<std::endl;
    std::cout<<src.getRows()<<std::endl;
    std::cout<<std::endl;

    // Hypothesis: The same memory is reassigned for resultMat when function is called again in the same scope + some weird behavior i haven't determined
    // Better Fix: Simply zeros each new Matrix
    // Need more Test
    // Temporaru patch for undetermined bug
    // resultMat.zeros();

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
    int a = src.getRows();
    int b = src.getCols();
    int c = src.getRows();

    writeImgPng("./UEB", resultMat, true);

    for (const Region &region : regions)
    {

        if (region.getSizePix() > 0)
        {
            int id = (region.pixels[0]->getId()) % 255;
            for (const std::shared_ptr<Pixl> &pixelptr : region.pixels)
            {
                resultMat.at(pixelptr->y, pixelptr->x)[0] = colorR[id];
                resultMat.at(pixelptr->y, pixelptr->x)[1] = colorG[id];
                resultMat.at(pixelptr->y, pixelptr->x)[2] = colorB[id];
            }
        }
    }

    writeImgPng("./UEB22", resultMat, true);

    return resultMat;
}

Mat<uint8_t> composeDisplayEdge(Mat<uint8_t> &visited, std::vector<Region> &regions)
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
int testregiongrowing(int argc, char **argv)
{
    int i;

    // Paramètres
    int seedquant = 100;
    float threshold = 5;
    // lecture des arguments

    const char *filepath = "0.jpg";

    Mat<u_char> img = loadImg(filepath, true);
    Mat<u_char> imgb = loadImg(filepath, true);

    std::vector<Region> regions;
    std::vector<Region> regionsb;

    std::queue<std::shared_ptr<Pixl>> seedlist = putGermsDivide(img, regions, 10);

    std::cout<<seedlist.size() <<"seedlist" << std::endl;
    std::queue<std::shared_ptr<Pixl>> seedlistb = putGermsDivide(imgb, regionsb, 10);

    // Mat that will be holding the different
    Mat<uint32_t> visited(img.getRows(), img.getCols(), 1);
    Mat<uint32_t> visitedb(img.getRows(), img.getCols(), 1);
    ;
    std::chrono::milliseconds durationa;
    std::chrono::milliseconds durationb;

    if (img.getChannels() == 3)
    {

        std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

        std::cout << "growing A " << std::endl;

        regionGrowing3c(img, seedlist, threshold, visited, regions);
        std::cout << "growing A done" << std::endl;
        fuseRegions(regions, threshold);
        std::chrono::high_resolution_clock::time_point stop = std::chrono::high_resolution_clock::now();

        durationa = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        /*fuseRegions(regions, threshold );

        start = std::chrono::high_resolution_clock::now();
        // Sharpened
        regionGrowing3c(imgb, seedlistb, threshold, visitedb, regionsb);


        fuseRegions(regionsb, threshold);
        fuseRegions(regionsb, threshold);

        stop = std::chrono::high_resolution_clock::now();
        durationb = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    }
    else if (img.getChannels() == 1)
    {

        regionGrowing1c(img, seedlist, threshold, visited, regions);
    }

    int taillerega = 0;
    for (int i = 0; i < regions.size(); i++) {
        //std::cout << regions[i].getSizePix() << "regions a with is first being " <<std::endl;

        // std::cout<<regions[i].pixels[0] -> getId() << regions[i].getMeanIntensity()<< std::endl;
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

    Mat<uint8_t> display = composeSegMean(img, regions);

    Mat<uint8_t> displayRandCol = composeSegRandCol(img, regions);

    writeImgPng("./First", display, true);
    writeImgPng("./secon", displayRandCol, true);

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





// test
void testSlic()
{
    SlicParameter param;
   // SlicAlgorithm<unsigned char>("0.jpg", param);
    // pipeline(data);
}

// Regionns wide modiication


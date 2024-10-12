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

    for (const Region &region : regions)
    {
        if (region.getSizePix() > 0)
        {
            for (const std::shared_ptr<Pixl> &pixelptr : region.pixels)
            {
                resultMat.atChannel(pixelptr->y, pixelptr->x, 0) = static_cast<uint8_t>(region.getMeanIntensity());
            }
        }
    }
    writeImgPng("./UEB1", resultMat, true);

    return resultMat;
}

// Create from the visited a representation of the segmentation in RGB

Mat<uint8_t> composeSegMeanColor(const Mat<uint8_t> &src, std::vector<Region> &regions)
{
    Mat<uint8_t> resultMat(src.getRows(), src.getCols(), 3);
    std::cout<<"INmea her" <<std::endl;

    for (const Region &region : regions)
    {
        if (region.getSizePix() > 0)
        {
            Vec<uint8_t> regmeanclr = (region.computeColorIntensity(src));

            for (const std::shared_ptr<Pixl> &pixelptr : region.pixels)
            {
                resultMat.at(pixelptr->y, pixelptr->x)[0] = regmeanclr.x ;
                resultMat.at(pixelptr->y, pixelptr->x)[1] = regmeanclr.y ;
                resultMat.at(pixelptr->y, pixelptr->x)[2] = regmeanclr.z ;
            }
        }
    }
    std::cout<<"INmea her" <<std::endl;
    
    writeImgPng("./UEB3", resultMat, true);

    return resultMat;
}
//TODO REmove the write here
// TODO 05/07/2024 Post GUI
// Unknog bud in SLic when this is outputted twice
Mat<uint8_t> composeSegRandCol(const Mat<uint8_t> &src, const std::vector<Region> &regions)
{
    Mat<uint8_t> resultMat(src.getRows(), src.getCols(), 3, PixlColorSpace::RGB);
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



#include <iostream>
#include "Mat.h"
#include <vector>
#include "Vec.h"

template <typename T>  
Mat<T> createMat3x3( float a1,float a2,float a3,float b1,float b2,float b3,float c1, float c2, float c3);

template <typename T>  
Mat<int> calcHistogram(const Mat<T>& img,  int bins = 256);

//void outputhist(const Mat<int>& canvas, const Mat<int>& hist, int histSize, int hist_h,int hist_w ,Vec<int> color);

Mat<int> normalizeHist(Mat<int>& tonormalize, int minrange, int maxrange) ;
 

Mat<int> stretchHist(const Mat<int>& img,int b, int a);
 
template <typename T>  
Mat<T> egalizeHist(const Mat<T>& img, int pow);


int findPeak( const Mat<float>& hist) ;


template <typename T>  
Mat<T> applyFilterConv3(const Mat<T>& src, const Mat<T>& filter) ;

template <typename T>  
Mat<T> applyFilterConv3C3(const Mat<T>& src, const Mat<T>& filter) ;


 
 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


template <typename T>
Mat<T> createMat3x3(float a1, float a2, float a3, float b1, float b2, float b3, float c1, float c2, float c3)
{
    Mat<T> mat(3, 3, 1);
    // Set elements individually
    mat.at(0, 0)[0] = a1;
    mat.at(1, 0)[0] = a2;
    mat.at(2, 0)[0] = a3;
    mat.at(0, 1)[0] = b1;
    mat.at(1, 1)[0] = b2;
    mat.at(2, 1)[0] = b3;
    mat.at(0, 2)[0] = c1;
    mat.at(1, 2)[0] = c2;
    mat.at(2, 2)[0] = c3;
    // Set all elements of the matrix to the given value
    // mat.setTo(cv::Scalar(value));
   

    return mat;

}

// Ok
template <typename T>
Mat<MT_INT> calcHistogram(const Mat<T> &img, int bins)
{
    // 256 by default
    Mat<int> hist(bins, 1, 1);
    for (int i = 0; i < img.getRows(); i++)
    {
        for (int j = 0; j < img.getCols(); j++)
        {
            hist.at(img.at(i, j), 0) += 1;
        }
    }
    return hist;
}

/*
Opencv based picture outputing
void outputhist(const cv::Mat& canvas, const cv::Mat& hist, int histSize, int hist_h,int hist_w , cv::Scalar color){
    int bin_w = cvRound((double) hist_w / histSize);
    for (int i = 1; i < histSize; i++) {
        line(canvas, cv::Point(bin_w * (i - 1),
        hist_h - cvRound(hist.at <float > (i - 1))),
        cv::Point(bin_w * (i), hist_h - cvRound(hist.at < float > (i))),
        color, 2, 8, 0);
    }
}*/

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

template <typename T>
// TODO
// Image might always be unsiched char, or at worst float
Mat<T> EgalizeHist(const Mat<T> &img, int power)
{

    int histSize = 256;
    float range[] = {
        0,
        256}; // the upper boundary is exclusive
    const float *histRange[] = {range};
    int hist_w = 512, hist_h = 400;
    int bin_w = std::round((float)(hist_w / histSize));

    // TODO, Trim useless part above
    Mat<int> hist = calcHistogram(img);

    // Comparaison Histograme normalisé

    for (int j = 1; j < hist.getRows(); j++)
    {
        //    cmlhist.at <float> (i) = cmlhist.at < float > (i - 1) + hist.at < float > (i);
        hist.atChannel(j, 0, 0) += hist.atChannel(j - 1, 0, 0);
    }

    // Comparaison Histograme normalisé
    // Old outputting of histImagea to compare both
    // cv::Mat histImagea(hist_h, hist_w, CV_8UC1, cv::Scalar(0, 0, 0));
    // TODO create graphic function
    // outputhist(histImagea, hist, histSize, hist_h, hist_w, Vec<int>(255, 0, 0));
    // cv::imshow("Histogramme fonction dansegalize pour Image Noir et blanc", histImagea);

    Mat<float> cdf = hist.template convertDt<float>();
    cdf /= (img.getRows() * img.getCols());
    // std::cout << cdf.getRows() << "colonnes" << cdf.getCols();
    Mat<T> newimg(img);

    for (int j = 0; j < img.getRows(); j++)
    {
        for (int i = 0; i < img.getCols(); i++)
        {
            for (int c = 0; c < img.getChannels(); j++)
            {
                int cdfpix = static_cast<int>(img.at(j, i, c));
                newimg.atChannel(j, i, c) = static_cast<T>(cdf.atChannel(cdfpix, 0, 0) * 255);
                // newimg.at<uchar>(i, j) = static_cast<uchar>((  (pow(2, power) - 1) * (cdf.at<float>(cdfpix) - cdf.at<float>(0)) ));
            }
        }
    }

    return newimg;
}

template <typename T>
Mat<T> applyFilterConv3(const Mat<T> &src, const Mat<T> &filter)
{
    Mat<T> newimg(src.getRows(), src.getCols(), src.getChannels());

    int filterSize = filter.getRows();

    for (int i = 1; i < src.getCols() - 1; ++i)
    {
        for (int j = 1; j < src.getRows() - 1; ++j)
        {
            for (int c = 0; c < src.getChannels(); ++c)
            {
                // Appliquer la convolution
                T sum = 0.0f;
                for (int k = 0; k < filterSize; ++k)
                {
                    for (int l = 0; l < filterSize; ++l)
                    {
                        //  if (i + k >= 0 && i + k< inputImage.getRows() && j + l >= 0 && j + l < inputImage.getCols()) {
                        sum += filter.atChannel(k, l, c) * src.atChannel(j + k - 1, i + l - 1, 0);
                        //  }
                    }
                }

                // Limiter la valeur entre 0 et 255
                // TODO maybe clamp
                sum = std::clamp(sum, static_cast<T>(0), static_cast<T>(255));

                // Modify current pixel
                newimg.atChannel(j, i, c) = (sum);
            }
        }
    }

    return newimg;
}

/*
template <typename T>
Mat<T> applyFilterConv3C3(const cv::Mat &src, const cv::Mat &filter)
{
    std::assert(src.getChannels() == 3 && filter.channels() == 1);

    cv::Mat newimg = src.clone();

    cv::Mat inputImage;
    src.convertTo(inputImage, CV_32FC3);
    newimg.convertTo(newimg, CV_32FC3);

    int filterSize = filter.getRows();

    for (int i = 1; i < inputImage.getRows() - 1; ++i)
    {
        for (int j = 1; j < inputImage.getCols() - 1; ++j)
        {
            // Apply convolution for each channel independently
            for (int channel = 0; channel < 3; ++channel)
            {
                double sum = 0.0;
                for (int k = 0; k < filterSize; ++k)
                {
                    for (int l = 0; l < filterSize; ++l)
                    {
                        sum += filter.at<float>(k, l) * inputImage.at<cv::Vec3f>(i + k - 1, j + l - 1)[channel];
                    }
                }

                // Limit the value between 0 and 255
                sum = std::min(255.0, std::max(0.0, sum));

                // Modify current pixel for each channel
                newimg.at<cv::Vec3f>(i, j)[channel] = static_cast<float>(sum);
            }
        }
    }

    newimg.convertTo(newimg, CV_8UC3);

    return newimg;
}
*/
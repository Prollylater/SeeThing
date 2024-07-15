#pragma once
// TODO TEST Conveert
// Everythinng is really useful
#include <iostream>
#include <cstring>
#include <limits>
#include <math.h>
#include "Vec.h"
#include <vector>
#include <stdexcept>
#include "StructParam.h"

// Light weight Mat class based on OpenCv structure
// The loathsome row Major order is conserved

// Forward declaration

// TODO Exceptio handling channels, bad arguments etc..

enum PixlColorSpace : int
{
    BGR,
    RGB,
    Lab,
    HSV,
    Gray
};

 

template <typename T>
class Mat
{
private:
    T *data;
    int rows;
    int cols;
    int channels;
    PixlColorSpace clrspace;
    // Spare me calculation of matrix
    // Avoid using this, shortcircuit as macro
    inline T &atChannelOp(int row, int col, int channel) const;

public:
    // Constructors
    Mat();
    Mat(int rows, int cols, int channels, PixlColorSpace clrspace = PixlColorSpace::RGB);
    Mat(T *_data, int _rows, int _cols, int _channels);
    Mat(const Mat &otherMat);

    Mat(Mat &&otherMat) noexcept;

    // Functions //
    int getRows() const { return rows; }
    const T *getData() const { return data; }

    int getCols() const { return cols; }
    //TODO finish this
    /* int resizedims(int _rows =this->rows,int _cols = this->cols,

     //TODO This
     int _channels= this->channels)  {
         //comparison with previous size
         rows = _rows;
         cols = _cols;
         channels = _channels;
         //new
      }*/

    int getChannels() const { return channels; };
    PixlColorSpace getClrspace() const { return clrspace; };
    void setClrSpace(PixlColorSpace newspace) { clrspace = newspace; };

    // Initialization
    void zeros();
    void setTo(T val);

    //  COLOR SPACE Conversion
    void toBGRSpace();
    void toRGBSpace();
    void toGRAYSpace();
    void toLabSpace(SlicParameter& conv_param);
    void toHsvSpace();

    // Operator //

    // TODO, overload or keep the non const ref pass
    //  Split a Matimg object according to it's number of channels
    // Passing channel as &
    void splitMat(std::vector<T> *channel1, std::vector<T> *channel2, std::vector<T> *channel3 = nullptr, std::vector<T> *channel4 = nullptr) const;
    void splitMat(Mat<T> *channels) const;

    // Ouptut local maximum and minimum of a 1 channel image
    void minMaxLoc(float &min, float &max) const;
    void minMaxLoc(int &min, int &max) const;
    // Maximum value of a single channel
    void minMaxLocChan(float &min, float &max, int channel) const;
    void minMaxLocChan(int &min, int &max, int channel) const;

    Mat<T> transpose() const;

    // TODO intensive operation to avoid redo
    template <typename T2>
    Mat<T2> convertDt() const
    {
        static_assert(std::is_convertible<T, T2>::value,
                      "Type used in convertDt not convertible to target type");
        T2 *newdata = new T2[rows * cols * channels];

        for (int i = 0; i < (rows * cols * channels); ++i)
        {
            newdata[i] = static_cast<T2>(this->data[i]);
        }
        Mat<T2> newmat(newdata, this->rows, this->cols, this->channels);

        return newmat;
    };

    template <typename T2>
    // TODO, overload or keep the non const ref pass
    friend void merge(Mat<T2> &tomerge, std::vector<T2> channel1, std::vector<T2> channel2, std::vector<T2> channel3, std::vector<T2> channel4);

    // Operator //

    // Assignment Op
    Mat<T> &operator=(const Mat<T> &other);

    Mat<T> &operator=(Mat<T> &&otherMat);

    // Calculation

    // Operator //
    T sumComponents() const;

    // Multiplication
    Mat<T> &operator*=(const Mat<T> &otherMat);
    Mat<T> &operator*=(const T &scalar);
    // Addition
    Mat<T> &operator+=(const Mat<T> &otherMat);
    Mat<T> &operator+=(const T &scalar);
    // Soustraction
    Mat<T> &operator-=(const Mat<T> &otherMat);
    Mat<T> &operator-=(const T &scalar);
    // Division
    Mat<T> &operator/=(const T &scalar);

    template <typename T2>
    friend Mat<T2> operator/(const Mat<T2> &Mata, const T2 &scalar);
    template <typename T2>
    friend Mat<T2> operator-(const Mat<T2> &Mata, const T2 &scalar);
    template <typename T2>
    friend Mat<T2> operator-(const Mat<T2> &Mata, const Mat<T2> &Matb);
    template <typename T2>
    friend Mat<T2> operator+(const Mat<T2> &Mata, const T2 &scalar);
    template <typename T2>
    friend Mat<T2> operator+(const Mat<T2> &Mata, const Mat<T2> &Matb);
    template <typename T2>
    friend inline Mat<T2> operator*(const Mat<T2> &Mata, const T2 &scalar);
    template <typename T2>
    friend inline Mat<T2> operator*(const Mat<T2> &Mata, const Mat<T2> &Matb);

    friend std::ostream &operator<<(std::ostream &os, const Mat<T> &mat)
    {
        int channels = mat.getChannels();

        int rows = mat.getRows();
        int cols = mat.getCols();

        for (int c = 0; c < channels; ++c)
        {
            os << "Channel :" << c << std::endl;
            for (int j = 0; j < rows; ++j)
            {
                for (int i = 0; i < cols; ++i)
                {
                    os << mat.atChannel(j, i, c) << " ";
                }
                os << std::endl;
            }
        }
        return os;
    }

    /*

    Inversion Matricielle

    Déterminant Matriciel

    Eigenvalue and Eigenvector Computation:
    */

    // Accessors
    T &atChannel(int row, int col, int channel);
    const T &atChannel(int row, int col, int channel) const;

    // TODO Define outside the at with inbuilt cast for atChannel
    template <typename T2>
    T2 atCast(int row, int col, int channel);
    template <typename T2>

    const T2 atCast(int row, int col, int channel) const;

    // TODO Convert it to std::array
    T *at(int row, int col);
    const T *at(int row, int col) const;

    // Return a vec object composed by the elements sharing the locatioon, row-col pass on the image
    Vec<T> atVec(int row, int col);
    const Vec<T> atVec(int row, int col) const;

    ~Mat()
    {
        delete[] data;
    }
};

// Merge a Matimg object splitted according to it's number of channels
template <typename T>
void merge(Mat<T> &tomerge, std::vector<T> channel1, std::vector<T> channel2, std::vector<T> channel3 = {}, const std::vector<T> channel4 = {});
template <typename T>
// Mat<T> mergeInto(std::vector<T> &channel1={}, std::vector<T> &channel2={}, std::vector<T> &channel3={}, std::vector<T> &channel4={});
Mat<T> mergeInto(std::vector<std::vector<T>> &channels, bool equalchannel = 0, int roww = 0, int coll = 0);

template <typename T>
Mat<T> operator/(const Mat<T> &Mata, const T &scalar);
template <typename T>
Mat<T> operator-(const Mat<T> &Mata, const T &scalar);
template <typename T>
Mat<T> operator-(const Mat<T> &Mata, const Mat<T> &Matb);
template <typename T>
Mat<T> operator+(const Mat<T> &Mata, const T &scalar);
template <typename T>
Mat<T> operator+(const Mat<T> &Mata, const Mat<T> &Matb);
template <typename T>
Mat<T> operator*(const Mat<T> &Mata, const T &scalar);
template <typename T>
Mat<T> operator*(const Mat<T> &Mata, const Mat<T> &Matb);

//////////////////////////////////////////////////////////////////////////////////////////
//
//                      TEMPLATE DEFINITION
//
//////////////////////////////////////////////////////////////////////////////////////////

// #include "Mat.h"

// Constructors
template <typename T>
Mat<T>::Mat() : data(nullptr), rows(0), cols(0), channels(0), clrspace(PixlColorSpace::RGB){
                                                                  // std::cout << "Creared fro noth" << std::endl;
                                                              };
template <typename T>
Mat<T>::Mat(int _rows, int _cols, int _channels, PixlColorSpace _clrspace) : rows(_rows), cols(_cols), channels(_channels), clrspace(_clrspace), data(nullptr)
{
    // std::cout << "Creared rows shit" << std::endl;
    data = new T[rows * cols * channels];
    this->zeros();
}

template <typename T>
Mat<T>::Mat(T *_data, int _rows, int _cols, int _channels) : rows(_rows),
                                                             cols(_cols), channels(_channels)
{
    // std::cout << "Creared from many shit" << std::endl;

    data = new T[rows * cols * channels];
    std::memcpy(data, _data, rows * cols * channels * sizeof(T));
    clrspace = (channels > 2) ? PixlColorSpace::RGB : PixlColorSpace::Gray;
    /* Potential managment of PixlColorSpace, Better use setter though
    if(channels > 2){
    clrspace = (channels == 4) ? PixlColorSpace::RGBA : PixlColorSpace::RGB};
    else{
        clrspace = (channels == 4) ? PixlColorSpace::RGBA : PixlColorSpace::RGB;
    }
    */
}

template <typename T>
Mat<T>::Mat(const Mat<T> &otherMat)
    : rows(otherMat.rows), cols(otherMat.cols), channels(otherMat.channels),
      clrspace(otherMat.clrspace)
{
    // std::cout << "Creared from copycons shit" << std::endl;

    data = new T[rows * cols * channels];
    std::memcpy(data, otherMat.data, rows * cols * channels * sizeof(T));
}

template <typename T>
Mat<T>::Mat(Mat<T> &&otherMat) noexcept
    : data(otherMat.data), rows(otherMat.rows),
      cols(otherMat.cols), channels(otherMat.channels)
{
    // std::cout << "Creared from assign shit" << std::endl;

    if (this != &otherMat)
    {
        clrspace = otherMat.clrspace;
        otherMat.data = nullptr;
        otherMat.rows = 0;
        otherMat.cols = 0;
    }
}

// Initialization
template <typename T>
void Mat<T>::zeros()
{   
    //TOTRY Benchmark this
    /*for (int i = 0; i < rows * cols * channels; ++i)
    {
        // Default constructor of each value
        data[i] = T();
    }
    */ 
    std::fill(data, data + rows * cols * channels, T{});

}

// Initialization
template <typename T>
void Mat<T>::setTo(T val)
{
    //TOTRY Benchmark this
    /*
    for (int i = 0; i < rows * cols * channels; ++i)
    {
        // Default constructor of each value
        data[i] = val;
    }*/
    std::fill(data, data + rows * cols * channels, val);

}

template <typename T>

inline T &Mat<T>::atChannelOp(int row, int col, int channel) const
{
    return data[(row * cols + col) * channels + channel];
}

// Split a Matimg object according to it's number of channels
template <typename T>
void Mat<T>::splitMat(std::vector<T> *channel1,
                      std::vector<T> *channel2, std::vector<T> *channel3,
                      std::vector<T> *channel4) const
{
    // Ensure the image has at elast 2 channels (GrayA)
    if (channels < 2)
    {
        std::cerr << "Image does not have at least 2 channels!" << std::endl;
        return;
    }

    // Create vectors to store individual channels
    channel1->resize(rows * cols);
    channel2->resize(rows * cols);
    if (channels >= 3)
    {
        channel3->resize(rows * cols);
    }
    if (channels == 4)
    {
        channel4->resize(rows * cols);
    }

    // Split the interleaved channels
    for (int i = 0; i < rows * cols; ++i)
    {
        (*channel1)[i] = data[i * channels + 0];
        (*channel2)[i] = data[i * channels + 1];
        if (channels >= 3)
        {
            (*channel3)[i] = data[i * channels + 2];
        }
        if (channels == 4)
        {
            (*channel4)[i] = data[i * channels + 3];
        }
    }

    return;
}

template <typename T>
void Mat<T>::splitMat(Mat<T> *channels) const
{
    // Ensure the image has at elast 2 channels (GrayA)
    int nbchannels = this->getChannels();

    if (nbchannels < 2)
    {
        std::cerr << "Image does not have at least 2 channels!" << std::endl;
        return;
    }
    std::vector<std::vector<T>> channel(4);

    splitMat(&channel[0], &channel[1], &channel[2], &channel[3]);

    for (int i = 0; i < this->getChannels(); i++)
    {
        channels[i] = Mat<T>(channel[i].data(), this->getRows(),
                             this->getCols(), 1);
    }

    return;
}

// Ouptut local maximum and minimum of a 1 channel image or simply the highest value
template <typename T>
void Mat<T>::minMaxLoc(float &min, float &max) const
{
    min = std::numeric_limits<float>::max();
    max = 0.0f;
    for (int i = 0; i < rows * cols * channels; ++i)
    {
        if (data[i] < min)
        {
            min = data[i];
        }
        if (data[i] > max)
        {
            max = data[i];
        }
    }
}
template <typename T>
void Mat<T>::minMaxLoc(int &min, int &max) const
{
    min = std::numeric_limits<int>::max();
    max = 0.0f;
    for (int i = 0; i < rows * cols * channels; ++i)
    {
        if (data[i] < min)
        {
            min = data[i];
        }
        if (data[i] > max)
        {
            max = data[i];
        }
    }
}

// TODO implement it
template <typename T>
void Mat<T>::minMaxLocChan(float &min, float &max, int channel) const
{
    min = std::numeric_limits<float>::max();
    max = 0.0f;
    for (int i = 0; i < rows * cols * channels; ++i)
    {
        if (data[i] < min)
        {
            min = data[i];
        }
        if (data[i] > max)
        {
            max = data[i];
        }
    }
}
template <typename T>
void Mat<T>::minMaxLocChan(int &min, int &max, int channel) const
{
    min = std::numeric_limits<int>::max();
    max = 0.0f;
    for (int i = 0; i < rows * cols * channels; ++i)
    {
        if (data[i] < min)
        {
            min = data[i];
        }
        if (data[i] > max)
        {
            max = data[i];
        }
    }
}
// Operator //
template <typename T>
T Mat<T>::sumComponents() const
{
    T sum = 0;
    for (int i = 0; i < rows * channels * cols; ++i)
    {
        sum += data[i];
    }
    return sum;
}

// Copy assignment Op
template <typename T>
Mat<T> &Mat<T>::operator=(const Mat<T> &othermat)
{
    if (this != &othermat)
    {
        delete[] data;

        rows = othermat.rows;
        cols = othermat.cols;
        clrspace = othermat.clrspace;
        channels = othermat.channels;

        data = new T[rows * cols * channels];
        std::memcpy(data, othermat.data, rows * cols * channels * sizeof(T));
    }

    return *this;
}

// Move assignment Op
template <typename T>
Mat<T> &Mat<T>::operator=(Mat<T> &&othermat)
{
    if (this != &othermat)
    {
        delete[] data;

        rows = othermat.rows;
        cols = othermat.cols;
        channels = othermat.channels;
        clrspace = othermat.clrspace;
        data = othermat.data;
        othermat.rows = 0;
        othermat.cols = 0;
        othermat.data = nullptr;
    }
    return *this;
}

// Calculation

// Operator //

// Multiplication
template <typename T>
Mat<T> &Mat<T>::operator*=(const Mat<T> &otherMat)
{
    if (this->channels != otherMat.channels || this->cols != otherMat.rows)
    {
        throw std::invalid_argument("Matrices dimensions or channel do not match for  assign multiplication.");
    }

    // data[(row * cols + col) * channels + channel]
    Mat<T> multmat(this->rows, this->cols, this->channels);

    // Perform matrix multiplication
    for (int c = 0; c < channels; ++c)
    {
        for (int j = 0; j < this->rows; ++j)
        {
            for (int i = 0; i < otherMat.cols; ++i)
            {
                for (int k = 0; k < this->cols; ++k)
                {
                    multmat.atChannelOp(j, i, c) += this->atChannelOp(j, k, c) * otherMat.atChannelOp(k, i, c);
                }
            }
        }
    }
    *this = multmat;
    return *this;
}

template <typename T>
Mat<T> &Mat<T>::operator*=(const T &scalar)
{ // TODO, make sure it don't mess up channel Alpha

    for (int i = 0; i < cols * rows * channels; ++i)
    {
        data[i] *= scalar;
    }
    return *this;
}
// Addition
template <typename T>
Mat<T> &Mat<T>::operator+=(const Mat<T> &otherMat)
{
    if (this->channels != otherMat.channels || this->rows != otherMat.rows || this->cols != otherMat.cols)
    {
        throw std::invalid_argument("Matrices dimensions or channel count do not match for assign element-wise addition.");
    }

    for (int c = 0; c < channels; ++c)
    {

        for (int j = 0; j < rows; ++j)
        {
            for (int i = 0; i < cols; ++i)
            {
                data[(j * cols + i) * channels + c] += otherMat.data[(j * cols + i) * channels + c];
            }
        }
    }
    return *this;
}

template <typename T>
Mat<T> &Mat<T>::operator+=(const T &scalar)
{
    for (int i = 0; i < cols * rows * channels; ++i)
    {
        data[i] += scalar;
    }
    return *this;
}

// Substraction
template <typename T>
Mat<T> &Mat<T>::operator-=(const Mat<T> &otherMat)
{
    if (this->channels != otherMat.channels || this->rows != otherMat.rows || this->cols != otherMat.cols)
    {
        throw std::invalid_argument("Matrices dimensions or channel count do not match for element-wise substration.");
    }

    for (int c = 0; c < channels; ++c)
    {

        for (int j = 0; j < rows; ++j)
        {
            for (int i = 0; i < cols; ++i)
            {
                data[(j * cols + i) * channels + c] -= otherMat.data[(j * cols + i) * channels + c];
            }
        }
    }
    return *this;
}

template <typename T>
Mat<T> &Mat<T>::operator-=(const T &scalar)
{
    for (int i = 0; i < cols * rows * channels; ++i)
    {
        data[i] -= scalar;
    }
    return *this;
}

template <typename T>
Mat<T> &Mat<T>::operator/=(const T &scalar)
{
    for (int i = 0; i < cols * rows * channels; ++i)
    {
        data[i] /= scalar;
    }
    return *this;
}

template <typename T>
Mat<T> Mat<T>::transpose() const
{
    Mat<T> transposed(this->cols, this->rows, this->channels);
    for (int c = 0; c < channels; ++c)
    {
        for (int j = 0; j < rows; ++j)
        {
            for (int i = 0; i < cols; ++i)
            {
                transposed.atChannelOp(i, j, c) = this->atChannelOp(j, i, c);
            }
        }
    }

    return transposed;
}

// Accessors
template <typename T>
T &Mat<T>::atChannel(int row, int col, int channel)
{
    if (row < 0 || row >= rows || col < 0 || col >= cols || channel < 0 || channel >= channels)
        throw std::out_of_range("Index out of bounds");
    return data[(row * cols + col) * channels + channel];
}

template <typename T>
const T &Mat<T>::atChannel(int row, int col, int channel) const
{
    if (row < 0 || row >= rows || col < 0 || col >= cols || channel < 0 || channel >= channels)
        throw std::out_of_range("Index out of bounds");
    return data[(row * cols + col) * channels + channel];
}

// TODO Amelioration ?

template <typename T>
template <typename T2>
T2 Mat<T>::atCast(int row, int col, int channel)
{
    if (row < 0 || row >= rows || col < 0 || col >= cols || channel < 0 || channel >= channels)
        throw std::out_of_range("Index out of bounds");
    return static_cast<T2>(data[(row * cols + col) * channels + channel]);
}
template <typename T>
template <typename T2>
const T2 Mat<T>::atCast(int row, int col, int channel) const
{
    if (row < 0 || row >= rows || col < 0 || col >= cols || channel < 0 || channel >= channels)
        throw std::out_of_range("Index out of bounds");
    return static_cast<T2>(data[(row * cols + col) * channels + channel]);
}

// TODO Convert it to std::array
template <typename T>
T *Mat<T>::at(int row, int col)
{
    if (row < 0 || row >= rows || col < 0 || col >= cols)
        throw std::out_of_range("Index out of bounds");
    return &data[(row * cols + col) * channels];
}

template <typename T>
const T *Mat<T>::at(int row, int col) const
{
    if (row < 0 || row >= rows || col < 0 || col >= cols)
        throw std::out_of_range("Index out of bounds");
    return &data[(row * cols + col) * channels];
}

template <typename T>
Vec<T> Mat<T>::atVec(int row, int col)
{
    if (row < 0 || row >= rows || col < 0 || col >= cols)
        throw std::out_of_range("Index out of bounds");

    T x = data[(row * cols + col) * channels];
    T y = channels > 1 ? data[(row * cols + col) * channels + 1] : T();
    T z = channels > 2 ? data[(row * cols + col) * channels + 2] : T();

    return Vec<T>(x, y, z);
}
template <typename T>
const Vec<T> Mat<T>::atVec(int row, int col) const
{
    if (row < 0 || row >= rows || col < 0 || col >= cols)
        throw std::out_of_range("Index out of bounds");

    T x = data[(row * cols + col) * channels];
    T y = channels > 1 ? data[(row * cols + col) * channels + 1] : T();
    T z = channels > 2 ? data[(row * cols + col) * channels + 2] : T();

    return Vec<T>(x, y, z);
}

// Merge a Mat object splitted according to it's number of channels
template <typename T>
void merge(Mat<T> &tomerge, std::vector<T> channel1,
           std::vector<T> channel2, std::vector<T> channel3,
           std::vector<T> channel4)
{

    int rows = tomerge.getRows();
    int cols = tomerge.getCols();
    int channels = tomerge.getChannels();
    if (channels < 2)
    {
        std::cerr << "Image does not have at least 2 channels!" << std::endl;
        return;
    }

    if (channels >= 3 || (channel4.size() != 0))
    {
        channel3.resize(rows * cols);
    }
    if (channels == 4 || (channel4.size() != 0))
    {
        channel4.resize(rows * cols);
    }
    T *tempmerge = new T[rows * cols * channels];

    // Merge into interleaved channels
    for (int i = 0; i < rows * cols; ++i)
    {
        tempmerge[i * channels + 0] = channel1[i];
        tempmerge[i * channels + 1] = channel2[i];
        if (channels >= 3)
        {
            tempmerge[i * channels + 2] = channel3[i];
        }
        if (channels == 4)
        {
            tempmerge[i * channels + 3] = channel4[i];
        }
    }
    tomerge.data = tempmerge;

    return;
}

template <typename T>
// Merge an unknow number of channels into a square matrix
// Padd matrix of smaller size with 0
Mat<T> mergeInto(std::vector<std::vector<T>> &channels, bool equalchannel, int roww, int coll)
{
    // TODO, check how variadic could improve the readiblity
    int max = 0;

    // If the check equalchannel is 1, refuse merge when all channel are not equal
    for (size_t i = 0; i < channels.size(); ++i)
    {
        if (channels[0].size() != channels[i].size())
        {
            if (equalchannel)
            { // TODO Exception
                Mat<T> newmat;
                return newmat;
            }
            break;
        }
    }

    // Else they will be padded later on
    for (std::vector<T> chan : channels)
    {
        int curr = chan.size();
        max = std::max(curr, max);
    }
    // if square
    // TODO determine how to best make the rows and cols
    int rows = std::sqrt(max);
    int cols = rows;
    int nbchannels = channels.size();

    // Handle no channels submitted or less
    if (nbchannels < 2)
    {
        if (nbchannels < 1)
        {
            // TODO Stuff to expection
            Mat<T> newmat;
            return newmat;
        }
        T *newdata = channels[0].data();
        Mat<T> newmat(newdata, rows, cols, nbchannels);
        return newmat;
    }

    for (std::vector<T> chan : channels)
    {

        chan.resize(max);
    }

    T *newdata = new T[rows * cols * nbchannels];

    std::cout << "resize" << rows << " " << max << std::endl;
    // Merge into interleaved channels
    int currchan = 0;
    for (std::vector<T> chan : channels)
    {
        for (int i = 0; i < rows * cols; ++i)
        {
            newdata[i * nbchannels + currchan] = chan[i];
        }

        currchan++;
    }

    Mat<T> newmat(newdata, rows, cols, nbchannels);
    return newmat;
}

//////////////////////////////////////////////////////////////////////////////////////////
//
//                      TEMPLATE DEFINITION
//
//////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
inline Mat<T> operator*(const Mat<T> &Mata, const Mat<T> &Matb)
{
    int channels = Mata.channels;
    int rows = Mata.rows;
    int cols = Mata.cols;

    if (channels != Matb.channels || cols != Matb.rows)
    {
        throw std::invalid_argument("Matrices dimensions or channel count do not match for  multiplication.");
    }

    // data[(row * cols + col) * channels + channel]
    Mat<T> multmat(Mata.rows, Matb.cols, Mata.channels);

    // Perform matrix multiplication
    for (int c = 0; c < channels; ++c)
    {
        for (int j = 0; j < Mata.rows; ++j)
        {
            for (int i = 0; i < Matb.cols; ++i)
            {
                for (int k = 0; k < Mata.cols; ++k)
                {
                    multmat.atChannelOp(j, i, c) += Mata.atChannelOp(j, k, c) * Matb.atChannelOp(k, i, c);
                }
            }
        }
    }
    return multmat;
}

template <typename T>
inline Mat<T> operator*(const Mat<T> &Mata, const T &scalar)
{ // TODO, make it sure it don't mess up channel Alpha
    Mat<T> multmat(Mata.rows, Mata.cols, Mata.channels);
    for (int i = 0; i < Mata.cols * Mata.rows * Mata.channels; ++i)
    {
        multmat.data[i] = Mata.data[i] * scalar;
    }
    return multmat;
}

// Addition
template <typename T>
Mat<T> operator+(const Mat<T> &Mata, const Mat<T> &Matb)
{
    int channels = Mata.channels;
    int rows = Mata.rows;
    int cols = Mata.cols;

    if (channels != Matb.channels || Mata.rows != Matb.rows || Mata.cols != Matb.cols)
    {
        throw std::invalid_argument("Matrices dimensions or channel count do not match for element-wise addition.");
    }
    Mat<T> addmat(Mata.rows, Mata.cols, Mata.channels);

    for (int c = 0; c < channels; ++c)
    {

        for (int j = 0; j < rows; ++j)
        {
            for (int i = 0; i < cols; ++i)
            {
                addmat.data[(j * cols + i) * channels + c] = Mata.data[(j * cols + i) * channels + c] + Matb.data[(j * cols + i) * channels + c];
            }
        }
    }
    return addmat;
}

template <typename T>

Mat<T> operator+(const Mat<T> &Mata, const T &scalar)
{
    Mat<T> addmat(Mata.rows, Mata.cols, Mata.channels);

    for (int i = 0; i < Mata.cols * Mata.rows * Mata.channels; ++i)
    {
        addmat.data[i] = Mata.data[i] + scalar;
    }
    return addmat;
}

// Substraction
template <typename T>
Mat<T> operator-(const Mat<T> &Mata, const Mat<T> &Matb)
{
    int channels = Mata.channels;
    int rows = Mata.rows;
    int cols = Mata.cols;
    if (channels != Matb.channels || Mata.rows != Matb.rows || Mata.cols != Matb.cols)
    {
        throw std::invalid_argument("Matrices dimensions or channel count do not match for element-wise substraction.");
    }
    Mat<T> submat(Mata.rows, Mata.cols, Mata.channels);

    for (int c = 0; c < channels; ++c)
    {

        for (int j = 0; j < rows; ++j)
        {
            for (int i = 0; i < cols; ++i)
            {
                submat.data[(j * cols + i) * channels + c] = Mata.data[(j * cols + i) * channels + c] - Matb.data[(j * cols + i) * channels + c];
            }
        }
    }
    return submat;
}

template <typename T>
Mat<T> operator-(const Mat<T> &Mata, const T &scalar)
{

    Mat<T> submat(Mata.rows, Mata.cols, Mata.channels);
    for (int i = 0; i < Mata.cols * Mata.rows * Mata.channels; ++i)
    {
        submat.data[i] = Mata.data[i] - scalar;
    }
    return submat;
}

template <typename T>
Mat<T> operator/(const Mat<T> &Mata, const T &scalar)
{
    Mat<T> divmat(Mata.rows, Mata.cols, Mata.channels);

    for (int i = 0; i < Mata.cols * Mata.rows * Mata.channels; ++i)
    {
        divmat.data[i] = Mata.data[i] / scalar;
    }
    return divmat;
}

/////  COLOR Space() Conversion
// TODO, Ensure that those function won't be called with bad value "string"
// Using a switch statement with the enum
// https://www.rroij.com/open-access/various-colour-spaces-and-colour-space-conversion-algorithms-44-48.pdf

// Gamma Correction value 
#define Gamma_C_V 0.04045
#define Gamma_C_D 12.92
// https://docs.opencv.org/3.4/de/d25/imgproc_color_conversions.html
// https://www.easyrgb.com/en/math.php
//  For RGB
float gamma_correctLab(double c);

// For XYZ
float functiontt(float c, float a = 7.787f, float b = (16.f / 116.f));

// Might change it in the end
//  Value maybe /100
// DL65 XYZ Reference
#define LAB_XN 109.850f
#define LAB_YN 100.0f
#define LAB_ZN 35.585f

/*XYZ references*/
typedef struct
{
    float x, y, z;
    const char *description;
} XYZRefT;

// TODO Look into how to better express this
typedef enum
{
    A2,
    B2,
    C2,
    D50_2,
    D55_2,
    D65_2,
    D75_2,
    E_2,
    F1_2
} XYZRefHandle;

static XYZRefT XYZrefs[] = {
    {109.850f, 100.0f, 35.585f, "Incandescent/tungsten (2°)"},
    {99.0927f, 100.0f, 85.313f, "Old direct sunlight at noon (2°)"},
    {98.074f, 100.0f, 118.232f, "Old daylight (2°)"},
    {96.422f, 100.0f, 82.521f, "ICC profile PCS (2°)"},
    {95.682f, 100.0f, 92.149f, "Mid-morning daylight (2°)"},
    {95.047f, 100.0f, 108.883f, "Daylight, sRGB, Adobe-RGB (2°)"},
    {94.972f, 100.0f, 122.638f, "North sky daylight (2°)"},
    {100.000f, 100.0f, 100.000f, "Equal energy (2°)"},
    {92.834f, 100.0f, 103.665f, "Daylight Fluorescent (2°)"},
};

// TODO Test result
// TODO control output Matrix
template <typename T>

//void Mat<T>::toLabSpace(bool divbool, int div_str, int xyzref )
void Mat<T>::toLabSpace(SlicParameter& conv_param)
{
    if (channels < 3)
    {
        return;
    }

    int idref = conv_param.m_XYZconv;
    float divide = conv_param.m_divbool ? conv_param.m_divstr : 1;
    int delta;
    switch (clrspace)
    {
    case PixlColorSpace::BGR:
        break;
    case PixlColorSpace::RGB:
        float X;
        float Y;
        float Z;
        for (int j = 0; j < this->rows; ++j)
        {
            for (int i = 0; i < this->cols; ++i)
            {
                // TODO TO specific function/MACro
                // TO XYZ

                float valr = 100.f * gamma_correctLab(static_cast<float>(this->atChannelOp(j, i, 0)) / 255.0f);
                float valg = 100.f * gamma_correctLab(static_cast<float>(this->atChannelOp(j, i, 1)) / 255.0f);
                float valb = 100.f * gamma_correctLab(static_cast<float>(this->atChannelOp(j, i, 2)) / 255.0f);
                X = 0.4124f * valr + 0.3575f * valg + 0.1804f * valb;
                X = X / (XYZrefs[idref].x / divide);
                Y = 0.2126f * valr + 0.7151f * valg + 0.0721f * valb;
                Y = Y / (XYZrefs[idref].y / divide);
                Z = 0.0193f * valr + 0.1192f * valg + 0.9505f * valb;
                Z = Z / (XYZrefs[idref].z / divide);
                // TO XYZ

                // refvalues
                // float tevaluate= 0.008856.f ;
                // float a=7.787f ;
                // float b= 16/116.f;
                // Difference betwween the two thing
                this->atChannelOp(j, i, 0) = static_cast<T>(116 * (functiontt(Y)) - 16); // X or Y + diffent formula
                this->atChannelOp(j, i, 1) = static_cast<T>(500 * (functiontt(X) - functiontt(Y)));
                this->atChannelOp(j, i, 2) = static_cast<T>(200 * (functiontt(Y) - functiontt(Z)));
            }
        }
        break;
    case PixlColorSpace::Lab:
        break;
    case PixlColorSpace::HSV:
        break;
    case PixlColorSpace::Gray:
        break;
    default:
        std::cout << "Unknown color." << std::endl;
        return;
    }

    this->clrspace = PixlColorSpace::Lab;
}

template <typename T>
void Mat<T>::toBGRSpace()
{
    if (channels < 3)
    {
        return;
    }
    switch (clrspace)
    {
    case PixlColorSpace::BGR:
        break;
    case PixlColorSpace::RGB:
        T V;
        break;
    case PixlColorSpace::Lab:
        break;
    case PixlColorSpace::HSV:
        break;
    case PixlColorSpace::Gray:
        break;
    default:
        std::cout << "Unknown color." << std::endl;
        break;
    }
    clrspace = PixlColorSpace::BGR;
}

template <typename T>
void Mat<T>::toRGBSpace()
{
    if (channels < 3)
    {
        return;
    }
    switch (clrspace)
    {
    case PixlColorSpace::BGR:
        break;
    case PixlColorSpace::RGB:

        break;
    case PixlColorSpace::Lab:
        break;
    case PixlColorSpace::HSV:
        break;
    case PixlColorSpace::Gray:
        break;
    default:
        std::cout << "Unknown color." << std::endl;
        break;
    }
    clrspace = PixlColorSpace::RGB;
}

template <typename T>
void Mat<T>::toGRAYSpace()
{
    if (channels < 3)
    {
        return;
    }
    switch (clrspace)
    {
    case PixlColorSpace::BGR:
        break;
    case PixlColorSpace::RGB:
        break;
    case PixlColorSpace::Lab:
        break;
    case PixlColorSpace::HSV:
        break;
    case PixlColorSpace::Gray:
        break;
    default:
        std::cout << "Unknown color." << std::endl;
        break;
    }
    clrspace = PixlColorSpace::Gray;
}

template <typename T>
void Mat<T>::toHsvSpace()
{
    if (channels < 3)
    {
        return;
    }
    Mat<T> newmat(this->rows, this->cols, this->channels);
    switch (clrspace)
    {
    case PixlColorSpace::HSV:
        break;
    case PixlColorSpace::RGB:
        T Vchannel;
        T Delta;
        T Gchannel;
        T Bchannel;

        for (int j = 0; j < this->rows; ++j)
        {
            for (int i = 0; i < this->cols; ++i)
            {
                T value0 = data.atChannelOp(j, i, 0);
                T value1 = data.atChannelOp(j, i, 1);
                T value2 = data.atChannelOp(j, i, 2);
                Vchannel = std::max({value0, value1, value2});
                Delta = std::min({value0, value1, value2});
                Delta = Vchannel - Delta;
            }
        }

        break;
    case PixlColorSpace::BGR:
        break;
    case PixlColorSpace::Lab:

        break;

    case PixlColorSpace::Gray:
        break;
    default:
        std::cout << "Unknown color." << std::endl;
        break;
    }
    clrspace = PixlColorSpace::HSV;
}

// Explicit instation
//  Mat<float> linkerfloat;
//  Mat<int> linkerint;


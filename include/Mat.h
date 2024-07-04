
// TODO TEST Conveert
// Everythinng is really useful
#include <iostream>
#include <cstring>
#include <limits>
#include <math.h>
#include "Vec.h"
#include <vector>
#include <stdexcept>

// Light weight Mat class based on OpenCv structure
// The loathsome row Major order is conserved

// Forward declaration

// TODO Exceptio handling channels, bad arguments etc..
#ifndef MAT_H
#define MAT_H
enum PixlColorSpace : unsigned short
{
    BGR,
    RGB,
    Lab,
    HSV,
    Gray
};

enum class DtType
{
    MT_UC,
    MT_UINT,
    MT_INT,
    MT_FLOAT,
};

typedef unsigned char MT_UC;
typedef unsigned int MT_UINT;
typedef int MT_INT;

typedef float MT_FLOAT;

template <DtType T>
struct PxlTraits;

// Specializations for each PxlDtType
template <>
struct PxlTraits<DtType::MT_UC>
{
    typedef unsigned char Type;
};

template <>
struct PxlTraits<DtType::MT_UINT>
{
    typedef unsigned int Type;
};

template <>
struct PxlTraits<DtType::MT_FLOAT>
{
    typedef float Type;
};
//    typedef typename PxlTraits<T>::Type DataType;

template <typename T>
class Mat
{
private:
    T *data;
    int rows;
    int cols;
    int channels;
    PixlColorSpace clrspace;
    DtType datatype;
    // Spare me calculation of matrix
    // Avoid using this, shortcircuit as macro
    inline T &atChannelOp(int row, int col, int channel) const;

public:
    // Constructors
    Mat();
    Mat(int rows, int cols, int channels, PixlColorSpace clrspace = PixlColorSpace::RGB);
    Mat(T *_data, int _rows, int _cols, int _channels);
    Mat(const Mat &otherMat);

    // TODO
    Mat(Mat &&otherMat) noexcept;

    // Functions //
    int getRows() const { return rows; }
    const T *getData() const { return data; }

    int getCols() const { return cols; }
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
    void toLabSpace();
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
    // TODO Convert it to std::array
    T *at(int row, int col);
    const T *at(int row, int col) const;

    Vec<T> atVec(int row, int col);
    const Vec<T> atVec(int row, int col) const;

    ~Mat()
    {
        delete (data);
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
Mat<T>::Mat() : data(nullptr), rows(0), cols(0), channels(0), clrspace(PixlColorSpace::RGB){};
template <typename T>
Mat<T>::Mat(int _rows, int _cols, int _channels, PixlColorSpace _clrspace) : rows(_rows), cols(_cols), channels(_channels), clrspace(_clrspace)
{
    data = new T[rows * cols * channels];
    // Not useful this->zeros(); ?
}

template <typename T>

Mat<T>::Mat(T *_data, int _rows, int _cols, int _channels) : rows(_rows),
                                                             cols(_cols), channels(_channels)
{
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
    : rows(otherMat.rows), cols(otherMat.cols), channels(otherMat.channels)
{
    data = new T[rows * cols * channels];
    std::memcpy(data, otherMat.data, rows * cols * channels * sizeof(unsigned char));
}

template <typename T>
Mat<T>::Mat(Mat<T> &&otherMat) noexcept
    : data(nullptr), rows(0), cols(0), channels(0)
{
    if (this != &otherMat)
    {
        rows = otherMat.rows;
        cols = otherMat.cols;
        channels = otherMat.channels;
        data = otherMat.data;
        otherMat.data = nullptr;
        otherMat.rows = 0;
        otherMat.cols = 0;
    }
}

// Initialization
template <typename T>
void Mat<T>::zeros()
{
    for (int i = 0; i < rows * cols * channels; ++i)
    {
        // Default constructor of each value
        data[i] = T();
    }
}

// Initialization
template <typename T>
void Mat<T>::setTo(T val)
{
    for (int i = 0; i < rows * cols * channels; ++i)
    {
        // Default constructor of each value
        data[i] = val;
    }
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
            (*channel3)[i] = data[i * channels + 3];
        }
        if (channels == 4)
        {
            (*channel4)[i] = data[i * channels + 3];
        }
    }

    return;
}

template <typename T>
void Mat<T>::splitMat(Mat<T>* channels) const
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

    for (int i = 0; i < getChannels; i++)
    {
        &(channels[i]) = new Mat(channel[i].data(), this->getRows(),
                         this->getCols(), this->getChannels());
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

// Assignment Op
template <typename T>
Mat<T> &Mat<T>::operator=(const Mat<T> &other)
{

    if (this != &other)
    {
        rows = other.rows;
        cols = other.cols;
        channels = other.channels;
        data = other.data;
    }

    return *this;
}

template <typename T>
Mat<T> &Mat<T>::operator=(Mat<T> &&otherMat)
{
    delete (data);
    if (this != &otherMat)
    {
        rows = otherMat.rows;
        cols = otherMat.cols;
        channels = otherMat.channels;
        data = std::move(otherMat.data);
        otherMat.rows = 0;
        otherMat.cols = 0;
        otherMat.data = nullptr;
    }
    return *this;
}

// Calculation

// Operator //

// Multiplication
template <typename T>
Mat<T> &Mat<T>::operator*=(const Mat<T> &otherMat)
{
    if (this->channels != otherMat.channels || this->rows != otherMat.rows || this->cols != otherMat.cols)
    {
        throw std::invalid_argument("Matrices dimensions or channel count do not match for element-wise multiplication.");
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
        throw std::invalid_argument("Matrices dimensions or channel count do not match for element-wise multiplication.");
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
        throw std::invalid_argument("Matrices dimensions or channel count do not match for element-wise multiplication.");
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

/*
template <typename T>
Mat<T> mergeInto(std::vector<T> &channel1, std::vector<T> &channel2, std::vector<T> &channel3, std::vector<T> &channel4){
{
    //TODO, check how variadic could improve the readiblity
    int rows = std::max(channel1.size(),channel2.size(), channel3.size(), channel4.size());
    rows = rows/2;
    int cols = rows;
    //Check  empty channel
    if(channel1.size() == 0)
    int channels = (channel1.size() == 0 +
                      channel2.size() == 0) +
                     (channel3.size() == 0 +
                      channel4.size() == 0);
    if (channels < 2)
    {
        std::cerr << "Image does not have at least 2 channels!" << std::endl;
        return;
    }

    if (channels >= 3)
    {
        channel3.resize(rows * cols);
    }
    if (channels == 4)
    {
        channel4.resize(rows * cols);
    }
    T *tempmerge = new T[rows * cols * channels];
    return;
}

}
*/

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

    if (channels != Matb.channels || rows != Matb.rows || cols != Matb.cols)
    {
        throw std::invalid_argument("Matrices dimensions or channel count do not match for element-wise multiplication.");
    }

    // data[(row * cols + col) * channels + channel]
    Mat<T> multmat(Mata.rows, Mata.cols, Mata.channels);

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
        multmat[i] = Mata.data[i] * scalar;
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
        throw std::invalid_argument("Matrices dimensions or channel count do not match for element-wise multiplication.");
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
        throw std::invalid_argument("Matrices dimensions or channel count do not match for element-wise multiplication.");
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

// For RGB
float gamma_correct(double c)
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
float functiontt(float c, float a = 7.787f, float b = 16 / 116.f)
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

// Might change it in the end
//  Value maybe /100
#define LAB_XN 95.0456
#define LAB_YN 100.0
#define LAB_ZN 108.8753

// TODO Test result
// TODO control output Matrix
template <typename T>
void Mat<T>::toLabSpace()
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
        T valr;
        T valb;
        T valg;
        float X;
        float Y;
        float Z;
        // https://docs.opencv.org/3.4/de/d25/imgproc_color_conversions.html
        // https://stackoverflow.com/questions/14175538/storing-lab-colors-as-integers
        for (int j = 0; j < this->rows; ++j)
        {
            for (int i = 0; i < this->cols; ++i)
            {
                // TODO TO specific function/MACro
                // TO XYZ
                valr = 100 * gamma_correct(data.atChannelOp(j, i, 0) / 255);
                valg = 100 * gamma_correct(data.atChannelOp(j, i, 1) / 255);
                valb = 100 * gamma_correct(data.atChannelOp(j, i, 2) / 255);
                X = 0.4124f * valr + 0.3575f * valg + 0.1804f * valb;
                X = X / LAB_XN;
                Y = 0.2126f * valr + 0.7151f * valg + 0.0721f * valb;
                Y = Y / LAB_YN;
                Z = 0.0193f * valr + 0.1191f * valg + 0.9502f * valb;
                Z = Z / LAB_ZN;
                // TO XYZ
                // refvalues
                // float tevaluate= 0.008856.f ;
                // float a=7.787f ;
                // float b= 16/116.f;
                // Difference betwween the two thing
                data.atChannelOp(j, i, 0) = 116 * functiontt(Y) - 16; // X or Y + diffent formula
                data.atChannelOp(j, i, 1) = 500 * (functiontt(X) - functiontt(Y));
                data.atChannelOp(j, i, 2) = 200 * (functiontt(Y) - functiontt(Z));
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
        break;
    }
    clrspace = PixlColorSpace::Lab;
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

#endif
#ifndef REGIONS_H
#define REGIONS_H

#include <iostream>

#include <cstdlib>

#include <algorithm> // std::random_shuffle

#include <thread>

#include <vector>

#include <map>

#include <mutex>

#include <chrono>

#include <limits>

#include <variant>
#include <functional>

#include <memory>

#include "Helper.h"

// TODO COntrol uneeded template
class Pixl;

template <typename T>
// Return as float the mean of the three channel value or the channel value of one Pixel
float getIntensity(const std::shared_ptr<Pixl> current,
                   const Mat<T> &image);

template <typename T>
//// Get color information of a Pixel. Expected Unsigned char and Float mainly
inline Vec<T> getPixlColor(const std::shared_ptr<Pixl> current,
                           const Mat<T> &image);

// TODO, There ought to be a better way... //FIxed
//  id being always resolved in a loop starting from 0, it is incremented by 1
//  the corrsponding region index in a regions vector is id-1
//  the corresponding indicatives in a matrix is id
class Pixl
{
public:
    int x;
    int y;

    int id;
    // Raise the id to better map to an img where Id 0 can't be used
    Pixl(int _x, int _y, int _id) : x(_x),
                                    y(_y),
                                    id(_id) {}
    void setId(int _id)
    {
        this->id = _id;
    }
    // TODO Change this
    //  Used to for easier mapping with vector object
    /*
    unsigned int getId()
    {
        int tmp = id - 1;
        return tmp;
    }*/
    int getId()
    {
        return id;
    }
};

// TODO decide how to handle private/public state
class Region
{
public:
    std::vector<std::shared_ptr<Pixl>> pixels; // Pixels belonging to the region
    std::vector<Region *> adjacentregions;     // List of adjacent regions
    float currentMeanIntensity;
    // Simple flag set after a fusion
    bool fusedaway = false;

    template <typename T>
    // Add a germ/pixel to a region
    // Img included to compute the Mean intensity on the go
    void addGerm(std::shared_ptr<Pixl> &pixeluptr, const Mat<T> &img);

    template <typename T>
    // Recompute the stored Mean Intensity
    float recomputeMeanIntensity(const Mat<T> &image);

    template <typename T>
    // Compute the mean intensity of every color in the region
    Vec<T> computeColorIntensity(const Mat<T> &image) const;
    Vec<uint8_t> computeColorIntensity(const Mat<uint8_t> &image) const;

    void fuse(Region *fusedin);

    void empty();

    void addAdjacentRegion(Region *region);

    // friend void fuseRegions(std::vector<Region> &regions, float threshold);

    // Accessors
    float getMeanIntensity() const;
    int getSizePix() const;
    int getState();
    void setFused();

    // Constructor
    Region(float _currentMeanIntensity = 0.0f); // Default Constructor
    Region(const Region &other);                // Copy constructor
    Region &operator=(const Region &other);     // Copy assignment operator
    Region(Region &&other);                     // Move constructor
    Region &operator=(Region &&other);          // Move assignment operator
    ~Region() = default;                        // CLeanup handled by Sharedptr
};

void fuseRegions(std::vector<Region> &regions, float threshold);

// -----------------------------------------------------
// ---------- SIMILARITY FUNCTION --------------
// -----------------------------------------------------

// TODO is template here  a good idea?
bool isSimilarMeanAverage(const Region &region1,
                          const Region &region2, float threshold);

bool isSimilarIntensity3c(const Mat<uint8_t> &bgrImage,
                          const std::shared_ptr<Pixl> &current,
                          const std::shared_ptr<Pixl> &neighbor, float threshold);

bool isSimilarIntensity3cGlobal(const Mat<uint8_t> &bgrImage,
                                const Region &region,
                                const std::shared_ptr<Pixl> &neighbor, float threshold);

bool isSimilarLuminance3cGlobal(const Mat<uint8_t> &bgrImage,
                                const Region &region,
                                const std::shared_ptr<Pixl> &neighbor, float threshold);

bool isSimilarEucDist(const Mat<uint8_t> &bgrImage,
                      const std::shared_ptr<Pixl> &current,
                      const std::shared_ptr<Pixl> &neighbor, float threshold);

template <typename T>
bool isSimilarIntensity1c(const Mat<T> &grayImage,
                          const std::shared_ptr<Pixl> &current,
                          const std::shared_ptr<Pixl> &neighbor, float threshold);

// TODO Change for map if there's ever any need to heavily extend this
// Enum is currently hardly used
enum class rgsCompFunc
{
    clr_intensity,
    eucld_dist,
    clr_intensity_reg,
    luminance_reg
};
// Using for template
// TODO Just use using everywhere ?

using CompPixPix = std::function<bool(const Mat<uint8_t> &,
                                      const std::shared_ptr<Pixl> &,
                                      const std::shared_ptr<Pixl> &, float)>;

using CompPixRegions = std::function<bool(const Mat<uint8_t> &,
                                          const Region &,
                                          const std::shared_ptr<Pixl> &, float)>;

using rgsCompFuncVar = std::variant<CompPixPix, CompPixRegions>;

////////////////////////////////////////////////////////////////////////////////////////////:

// TODO Reconsider similarity and intensity/engergy usage
//  PIXL RELATED
template <typename T>
float getIntensity(const std::shared_ptr<Pixl> current,
                   const Mat<T> &image)
{
    float intensity = 0;
    if (current->x < 0 || current->y < 0 || current->x >= image.getCols() || current->y >= image.getRows())
    {
        return intensity;
    }
    if (image.getChannels() > 2)
    {
        Vec<T> currentintensity = image.atVec(current->y, current->x);
        // L'intensité prend en compte les trois couleurs
        intensity = static_cast<float>(currentintensity.sumComponents() / 3);
    }
    else if (image.getChannels() == 1)
    {

        T currentintensity = image.atChannel(current->y, current->x, 0);
        // L'intensité prend en compte la seule valeur présente
        intensity = static_cast<float>(currentintensity); // favs
    }
    return intensity;
}

template <typename T>
inline Vec<T> getPixlColor(const std::shared_ptr<Pixl> current,
                           const Mat<T> &image)
{
    Vec<T> colorvec = image.atVec(current->y, current->x);
    return colorvec;
}

////////////////////////////////////////////////////////////////////////////
//                      Region function definition                       //
///////////////////////////////////////////////////////////////////////////

template <typename T>
void Region::addGerm(std::shared_ptr<Pixl> &pixeluptr, const Mat<T> &img)
{
    if (!pixeluptr)
    {
        std::cerr << "Error: Invalid pixel pointer!" << std::endl;
        return;
    }

    float tmpmean = currentMeanIntensity * pixels.size();
    tmpmean += getIntensity(pixeluptr, img);

    pixels.push_back(std::move(pixeluptr));
    currentMeanIntensity = tmpmean / pixels.size();
}

template <typename T>
float Region::recomputeMeanIntensity(const Mat<T> &image)
{
    float sumIntensity = 0.0;
    for (const std::shared_ptr<Pixl> &pixel : pixels)
    { // TODO Won't
        // Acess only B for a three channel
        sumIntensity += getIntensity(pixel, image);
    }
    return sumIntensity / pixels.size();
}

// Work and int, not so much with a type as limited as uint_8
// Or divide ach time
//  TODO Rethink those function and
template <typename T>
Vec<T> Region::computeColorIntensity(const Mat<T> &image) const
{
    Vec<T> sumIntensity;

    for (const std::shared_ptr<Pixl> &pixel : pixels)
    {
        sumIntensity += image.atVec(pixel->y, pixel->x);
    }

    sumIntensity /= pixels.size();

    return sumIntensity;
}

template <typename T>
bool isSimilarColorAverage(const Mat<T> &bgrImage, const Region &region1,
                           const Region &region2, float threshold)
{
    if (region1.getSizePix() == 0 || region2.getSizePix() == 0)
    {
        return false;
    }

    float intensitydiff = std::abs(region1.computeColorIntensity(bgrImage) - region2.computeColorIntensity(bgrImage)); // favs
    return (intensitydiff <= (threshold * 3));
}

// TODO May be useless with my mat
template <typename T>
bool isSimilarIntensity1c(const Mat<T> &grayImage,
                          const std::shared_ptr<Pixl> &current,
                          const std::shared_ptr<Pixl> &neighbor, float threshold)
{
    float currentintensity = getIntensity(current, grayImage);
    float neighborintensity = getIntensity(neighbor, grayImage);

    // Difference in intensity between neighbor and pixel considering a grayscale image
    int intensitydiff = ((currentintensity) - (neighborintensity));
    intensitydiff = (intensitydiff < 0) ? -intensitydiff : intensitydiff;

    return (intensitydiff <= (threshold)); //
}

#endif
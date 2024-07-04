#include <iostream>

#include <cstdlib>

#include <ctime>

#include <algorithm> // std::random_shuffle

#include <thread>

#include <queue>

#include <vector>

#include <map>

#include <mutex>

#include <chrono>

#include <random>

#include <limits>

#include "./include/Helper.hpp"

//TODO COntrol uneeded template
class Pixl;

template <typename T>
//Return as float the mean of the three channel value or the channel value of one Pixel
float getIntensity(const std::shared_ptr<Pixl> current,
                   const Mat<T> &image);

template <typename T>     
//// Get color information of a Pixel. Expected Unsigned char and Float mainly   
inline Vec<T> getPixlColor(const std::shared_ptr<Pixl> current,
                       const Mat<T> &image);


//TODO, There ought to be a better way...
// id being always resolved in a loop starting from 0, it is incremented by 1
// the corrsponding region index in a regions vector is id-1
// the corresponding indicatives in a matrix is id
class Pixl
{
public:
    unsigned short x;
    unsigned short y;
   
    unsigned int id;
    //Raise the id to better map to an img where Id 0 can't be used
    Pixl(unsigned short _x, unsigned short _y, unsigned short _id) : x(_x),
                                    y(_y),
                                    id(_id + 1) {}
    void setId(unsigned int _id)
    {
        this->id = _id + 1;
    }
    //TODO Change this
    // Used to for easier mapping with vector object
    unsigned int getIdInVec()
    {
        int tmp = id - 1;
        return tmp;
    }
    unsigned int getIdInImg()
    {
        return id;
    }

};

class Region
{
public:
    std::vector<std::shared_ptr<Pixl>> pixels; // Pixels belonging to the region
    std::vector<Region *> adjacentregions;     // List of adjacent regions

    float currentMeanIntensity;
    // Simple flag set after a fusion
    bool fusedaway = false;

    template<typename T>
    //Add a germ/pixel to a region
    //Img included to compute the Mean intensity on the go
    void addGerm(std::shared_ptr<Pixl> &pixeluptr,const Mat<T> &img);

    template<typename T>
    //Recompute the stored Mean Intensity
    float recomputeMeanIntensity(const Mat<T> &image);

    template<typename T>
    //Compute the mean intensity of every color in the region
	Vec<T> computeColorIntensity(const Mat<T> &image);

    void fuse(Region *fusedin);

    void addAdjacentRegion(Region *region);

    friend void fuseRegions(std::vector<Region> &regions, float threshold);

    //Accessors 
    float getMeanIntensity() const ;
    int getSizePix() const;
    int getState();
    void setFused();

    //Constructor
    Region(float _currentMeanIntensity = 0); //Default Constructor
    Region(const Region& other);            // Copy constructor
    Region& operator=(const Region& other); // Copy assignment operator
    Region(Region&& other) ;        // Move constructor
    Region& operator=(Region&& other) ; // Move assignment operator
   	~Region() = default; //CLeanup handled by Sharedptr
};





// -----------------------------------------------------
// ---------- SIMILARITY FUNCTION --------------
// -----------------------------------------------------

bool isSimilarMeanAverage(const Region &region1,
                          const Region &region2, float threshold);

template<typename T>
bool isSimilarIntensity3c(const Mat<T> &bgrImage,
                          const std::shared_ptr<Pixl> &current,
                          const std::shared_ptr<Pixl> &neighbor, float threshold);

template<typename T>
bool isSimilarIntensity3cGlobal(const Mat<T> &bgrImage,
                                const Region &region,
                                const std::shared_ptr<Pixl> &neighbor, float threshold);


template<typename T>
bool isSimilarLuminance3cGlobal(const Mat<T> &bgrImage,
                                 const Region &region,
                                 const std::shared_ptr<Pixl> &neighbor,float threshold);

template<typename T>
bool isSimilarEucDist(const Mat<T> &bgrImage,
                          const std::shared_ptr<Pixl> &current,
                          const std::shared_ptr<Pixl> &neighbor, float threshold);

template<typename T>
bool isSimilarIntensity1c(const Mat<T> &grayImage,
                          const std::shared_ptr<Pixl> &current,
                          const std::shared_ptr<Pixl> &neighbor, float threshold);



////////////////////////////////////////////////////////////////////////////////////////////:



//TODO Reconsider similarity and intensity/engergy usage
// PIXL RELATED 
template <typename T>
float getIntensity(const std::shared_ptr<Pixl> current,
                   const Mat<T> &image)
{
    float intensity = 0;
    if (image.getChannels() > 2)
    {
        Vec<T> currentintensity = image.atVec(current->x, current->y);
        // L'intisité prend en compte les trois couleurs
        intensity = currentintensity.sumComponents() / 3;
    }
    else if (image.getChannels() == 1)
    {
        T currentintensity = image.atChannel(current->x, current->y, 0);
        // L'intisité prend en compte la seule valeur présente
        intensity = static_cast<float>(currentintensity); // favs
    }
    return (intensity);
}

template <typename T>           
inline Vec<T> getPixlColor(const std::shared_ptr<Pixl> current,
                       const Mat<T> &image)
{
    Vec<T> colorvec = image.atVec<Vec<T>(current->x, current->y);
    return colorvec;
}


  //////////////////////////////////////////////////////////////////////////// 
 //                      Region function definition                       //
///////////////////////////////////////////////////////////////////////////

    template<typename T>
	void Region::addGerm(std::shared_ptr<Pixl> &pixeluptr,const Mat<T> &img)
	{
		this->currentMeanIntensity += (getintensity(pixeluptr, img)/pixels.size());
		pixels.push_back(std::move(pixeluptr));
	}

    template<typename T>
	float Region::recomputeMeanIntensity(const Mat<T> &image)
	{
		float sumIntensity = 0.0;
		for (const std::shared_ptr<Pixl> &pixel : pixels)
		{   //TODO Won't 
			// Acess only B for a three channel
			//sumIntensity += static_cast<float>(image.at<T>(pixel->x, pixel->y));
			sumIntensity += getintensity(pixel, image);
		
        }
		return sumIntensity / pixels.size();
	}

	template<typename T>
	Vec<T> Region::computeColorIntensity(const Mat<T> &image)
	{
		Vec<T> sumIntensity;
		for (const std::shared_ptr<Pixl> &pixel : pixels)
		{
			/*sumIntensity.x += image.atVec(pixel->x, pixel->y);
			sumIntensity.y += image.atVec(pixel->x, pixel->y);
			sumIntensity.z += image.atVec(pixel->x, pixel->y);*/
		    sumIntensity += image.atVec(pixel->x, pixel->y);
		}
		sumIntensity /= pixels.size();
		return sumIntensity;
	}

	void Region::fuse(Region *fusedin)
	{
		if (fusedin && fusedin != this)
		{

			// recalculating Meanintensity
			double tmp = this->currentMeanIntensity * this->pixels.size();
			double tmpfusedin = fusedin->getMeanIntensity() * fusedin->getSizePix();

			// std::cout << tmp << "for " << currentMeanIntensity << "of " << pixels.size() << std::endl;
			//  std::cout << tmpfusedin << "for " << fusedin->getMeanIntensity() << "of " << fusedin->getSizePix() << std::endl;

			pixels.reserve(pixels.size() + fusedin->pixels.size());

			// Fusing the pixel
			for (std::shared_ptr<Pixl> &pixelptr : fusedin->pixels)
			{
				pixelptr->setId(this->pixels[0]->getIdInVec());
			}

			for (std::shared_ptr<Pixl> &pixelptr : fusedin->pixels)
			{
				pixels.push_back((pixelptr));
				// pixels.push_back((std::move(pixelptr))); cause segfault
			}

			fusedin->pixels.clear();

			// Fusing the mean
			currentMeanIntensity = (tmp + tmpfusedin) / 2;

			adjacentregions.reserve(this->adjacentregions.size() + fusedin->adjacentregions.size());

			for (Region *reg : fusedin->adjacentregions)
			{
				this->addAdjacentRegion(reg);
			}

			// Fusing the adjacent regions

			if (!(std::find(adjacentregions.begin(), adjacentregions.end(), fusedin) == adjacentregions.end()))
			{
				adjacentregions.erase(std::remove(adjacentregions.begin(), adjacentregions.end(), fusedin), adjacentregions.end());
			}

			fusedin->adjacentregions.clear();

			//fusedin->adjacentregions.shrink_to_fit();
		}
	}

	void Region::addAdjacentRegion(Region *region)
	{
		// Prevent duplicate or adding additional region
		if (region)
		{
			if (std::find(adjacentregions.begin(), adjacentregions.end(), region) == adjacentregions.end() && this != region)
			{
				// If not in the list, add it
				this->adjacentregions.push_back((region));
			}
		}
	}

	float Region::getMeanIntensity() const
	{
		return currentMeanIntensity;
	}

	int Region::getSizePix() const
	{

		return pixels.size();
	}

	int Region::getState()
	{
		return fusedaway;
	}
	void Region::setFused()
	{
		this->fusedaway = true;
	}



// Constructor, nothing special
// Default constructor
Region::Region(float _currentMeanIntensity ) 
    : currentMeanIntensity(_currentMeanIntensity), fusedaway(false) {}

// Copy constructor
Region::Region(const Region& other) 
    : pixels(other.pixels), 
      adjacentregions(other.adjacentregions), 
      currentMeanIntensity(other.currentMeanIntensity), 
      fusedaway(other.fusedaway) {}

// Copy assignment operator
Region& Region::operator=(const Region& other) {
    if (this != &other) {
        pixels = other.pixels;
        adjacentregions = other.adjacentregions;
        currentMeanIntensity = other.currentMeanIntensity;
        fusedaway = other.fusedaway;
    }
    return *this;
}

// Move constructor
Region::Region(Region&& other)  
    : pixels(std::move(other.pixels)), 
      adjacentregions(std::move(other.adjacentregions)), 
      currentMeanIntensity(other.currentMeanIntensity), 
      fusedaway(other.fusedaway) {}

// Move assignment operator
Region& Region::operator=(Region&& other)  {
    if (this != &other) {
        pixels = std::move(other.pixels);
        adjacentregions = std::move(other.adjacentregions);
        currentMeanIntensity = other.currentMeanIntensity;
        fusedaway = other.fusedaway;
        other.currentMeanIntensity = 0;
        other.fusedaway = false;
    }
    return *this;
}


//TODO Remove it from friend ?

void fuseRegions(std::vector<Region> &regions, float threshold)
{
    int index = 0;
    std::vector<Region *> newregionsbuffer;

    // Fuse the current region with the region of the visited pixel
    // for (Region &regionA : regions)
    for (int i = 0; i < regions.size(); ++i)
    {

        if (!(regions[i].getState()) && !regions[i].adjacentregions.empty() && !regions[i].pixels.empty())
        {

            // for (Region *reg : regionA.adjacentregions) //ITerator shitting here
            for (int j = 0; j < regions[i].adjacentregions.size(); ++j)
            {
                Region *reg = regions[i].adjacentregions[j];
                if (!reg->getState())
                {
                    if (reg->pixels.size() > 0 && isSimilarMeanAverage(regions[i], *reg, threshold))
                    {
                        reg->setFused();
                        regions[i].fuse(reg);

                        // newregionsbuffer.push_back(reg);
                    }
                }
            }
        }
    }
}



// -----------------------------------------------------
// ---------- SIMILARITY FUNCTION --------------
// -----------------------------------------------------

//TODO REthink those function
//Threshold for each channel ? For one ? Etc...
bool isSimilarMeanAverage(const Region &region1,
                          const Region &region2, int threshold)
{
    if(region1.getSizePix() == 0 || region2.getSizePix() == 0){ return false;}
    float intensitydiff = std::abs(region1.getMeanIntensity() - region2.getMeanIntensity()); // favs
    return (intensitydiff <= (threshold * 3));
}

template<typename T>
bool isSimilarIntensity3c(const Mat<T> &bgrImage,
                          const std::shared_ptr<Pixl> &current,
                          const std::shared_ptr<Pixl> &neighbor, float threshold)
{
    //TODO Test
    Vec<float> currentintensity =getPixlColor(current,bgrImage);
    Vec<float> neighborintensity =getPixlColor(neighbor,bgrImage);

    float intensitydiff = 0;
    Vec<float> intdif =(currentintensity - neighborintensity);
     intdif.x= (intdif.x < 0) ? -intdif.x : intdif.x;
     intdif.y= (intdif.y < 0) ? -intdif.y : intdif.y;
     intdif.z= (intdif.z < 0) ? -intdif.z : intdif.z;

    intensitydiff = intdif.sumComponents() / 3;

    return (intensitydiff <= (threshold));
}


template<typename T>
bool isSimilarIntensity3cGlobal(const Mat<T> &bgrImage,
                                const Region &region,
                                const std::shared_ptr<Pixl> &neighbor, int threshold)
{
    Vec<T> neighborintensity = bgrImage.atVec(neighbor->x, neighbor->y);
    // Différence d'intensité entre voisin et pixel en prenant en compte les trois couleurs
    float totalintensity = neighborintensity.sumComponents()/3;
    return (std::abs((totalintensity) - region.getMeanIntensity())) < threshold;
   
}


template<typename T>
bool isSimilarEucDist(const Mat<T> &bgrImage,
                          const std::shared_ptr<Pixl> &current,
                          const std::shared_ptr<Pixl> &neighbor, float threshold)
{
    //TODO Test
    Vec<float>  eucldist = std::pow(
        (bgrImage.atVec(current->x, current->y) -
         bgrImage.atVec(neighbor->x, neighbor->y)), 2);

    // Compare the intensity difference with the threshold ou
   
    return (std::sqrt(eucldist.sumComponents())<= (threshold));

}


template<typename T>
bool isSimilarLuminance3cGlobal(const Mat<T> &bgrImage,
                                 const Region &region,
                                 const std::shared_ptr<Pixl> &neighbor,
                                 float threshold)
{
    Vec<T> neighborIntensity = bgrImage.at<Vec<T>(neighbor->x, neighbor->y);
    float weightedIntensity = 0;
    // Calculate luminance-weighted intensity BGR
    if(bgrImage.clrspace == PixlColorSpace::RGB){
         weightedIntensity = 0.299 * neighborIntensity.x + 0.587 * neighborIntensity.y + 0.114 * neighborIntensity.z;
    }
    else if(bgrImage.clrspace == PixlColorSpace::BGR){
         weightedIntensity = 0.299 * neighborIntensity.z + 0.587 * neighborIntensity.y + 0.114 * neighborIntensity.x;
    }
    else {
        return false;
    }
    // Compare the intensity difference with the threshold
    return std::abs(weightedIntensity - region.getMeanIntensity()) < threshold;
}


template<typename T>
bool isSimilarIntensity1c(const Mat<T> &grayImage,
                          const std::shared_ptr<Pixl> &current,
                          const std::shared_ptr<Pixl> &neighbor, float threshold)
{
    float currentintensity = getIntensity(current,grayImage);
    float neighborintensity = getIntensity(neighbor,grayImage);

    // Difference in intensity between neighbor and pixel considering a grayscale image
    int intensitydiff= ((currentintensity) - (neighborintensity));
    intensitydiff =  (intensitydiff < 0) ? -intensitydiff : intensitydiff;

    return (intensitydiff <= (threshold)); //
}















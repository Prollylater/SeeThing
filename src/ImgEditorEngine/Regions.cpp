#include "Regions.h"




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



 


  //////////////////////////////////////////////////////////////////////////// 
 //                      Region function definition                       //
///////////////////////////////////////////////////////////////////////////

  
    void Region::empty(){
        this->adjacentregions.clear();
        this->pixels.clear();
        currentMeanIntensity = 0;
        fusedaway = false;
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
				pixelptr->setId(this->pixels[0]->getId());
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
                Region * reg = regions[i].adjacentregions[j];
                if (!reg->getState())
                {
                    if (reg->pixels.size() > 0 && 
                    isSimilarMeanAverage(regions[i], *reg, threshold))
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
//Control whetehr or not two regions mean value are close enough 

bool isSimilarMeanAverage(const Region &region1,
                          const Region &region2, float threshold)
{
    if(region1.getSizePix() == 0 || region2.getSizePix() == 0){ return false;}
    float intensitydiff = std::abs(region1.getMeanIntensity() - region2.getMeanIntensity()); // favs
    return (intensitydiff <= (threshold * 3));
}








 
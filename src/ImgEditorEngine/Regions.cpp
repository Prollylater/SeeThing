#include "Regions.h"

// Constructor, nothing special
// Default constructor
Region::Region(float _currentMeanIntensity)
	: currentMeanIntensity(_currentMeanIntensity), fusedaway(false) {}

// Copy constructor
Region::Region(const Region &other)
	: pixels(other.pixels),
	  adjacentregions(other.adjacentregions),
	  currentMeanIntensity(other.currentMeanIntensity),
	  fusedaway(other.fusedaway) {}

// Copy assignment operator
Region &Region::operator=(const Region &other)
{
	if (this != &other)
	{
		pixels = other.pixels;
		adjacentregions = other.adjacentregions;
		currentMeanIntensity = other.currentMeanIntensity;
		fusedaway = other.fusedaway;
	}
	return *this;
}

// Move constructor
Region::Region(Region &&other)
	: pixels(std::move(other.pixels)),
	  adjacentregions(std::move(other.adjacentregions)),
	  currentMeanIntensity(other.currentMeanIntensity),
	  fusedaway(other.fusedaway) {}

// Move assignment operator
Region &Region::operator=(Region &&other)
{
	if (this != &other)
	{
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

void Region::empty()
{
	this->adjacentregions.clear();
	this->pixels.clear();
	currentMeanIntensity = 0;
	fusedaway = false;
}

void Region::fuse(Region *fusedin)
{
	if (!fusedin || fusedin == this)
	{
		return;
	}

	// Values of the current mean intensity
	float tmp = this->currentMeanIntensity * this->pixels.size();
	float tmpfusedin = fusedin->getMeanIntensity() * fusedin->getSizePix();

	pixels.reserve(pixels.size() + fusedin->pixels.size());

	// Changing the pixel id
	int newid = this->pixels[0]->getId();

	for (std::shared_ptr<Pixl> &pixelptr : fusedin->pixels)
	{
		pixelptr->setId(newid);
	}

	/*for (std::shared_ptr<Pixl> &pixelptr : fusedin->pixels)
	{
		pixels.push_back((pixelptr));
	}*/

	pixels.insert(pixels.end(),
				  std::make_move_iterator(fusedin->pixels.begin()),
				  std::make_move_iterator(fusedin->pixels.end()));

	// Recompute new mean
	currentMeanIntensity = (tmp + tmpfusedin) / pixels.size();

	// Handlign adjacent regions
	adjacentregions.reserve(this->adjacentregions.size() + fusedin->adjacentregions.size());

	for (Region *reg : fusedin->adjacentregions)
	{
		this->addAdjacentRegion(reg);
	}

	// Fusing the adjacent regions

	// Erase remove idiom to make sure eveyrhing is cleaned,
	// May be too much

	/*if (!(std::find(adjacentregions.begin(), adjacentregions.end(), fusedin) == adjacentregions.end()))
	{
		adjacentregions.erase(std::remove(adjacentregions.begin(), adjacentregions.end(), fusedin), adjacentregions.end());
	}*/

	// CLeaning the fused vector
	fusedin->pixels.clear();
	fusedin->setFused();
	fusedin->adjacentregions.clear();
	fusedin->adjacentregions.shrink_to_fit();
	fusedin->currentMeanIntensity = 0.0;

	// fusedin->adjacentregions.shrink_to_fit();
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

void fuseRegions(std::vector<Region> &regions, float threshold)
{
	int index = 0;
	std::vector<Region *> newregionsbuffer;
	// Fuse the current region with the region of the visited pixel
	// for (Region &regionA : regions)
	for (int i = 0; i < regions.size(); ++i)
	{

		// If it was fused at somepoint
		if ((regions[i].getState()) ||
			(regions[i].pixels.empty()) ||
			regions[i].adjacentregions.empty())
		{
			continue;
		}
		// Loop through all the adjacent regions
		// Possible index invalidation
		for (int j = 0; j < regions[i].adjacentregions.size(); ++j)
		{
			Region *reg = regions[i].adjacentregions[j];
			// If already fused

			if (reg->getState())
			{
				continue;
			}

			if (reg->pixels.size() > 0 &&
				isSimilarMeanAverage(regions[i], *reg, threshold))
			{
				regions[i].fuse(reg);

				// newregionsbuffer.push_back(reg);
			}
		}
	}
}

// -----------------------------------------------------
// ---------- SIMILARITY FUNCTION --------------
// -----------------------------------------------------

// TODO REthink those function
// Threshold for each channel ? For one ? Etc...
// Control whetehr or not two regions mean value are close enough

bool isSimilarMeanAverage(const Region &region1,
						  const Region &region2, float threshold)
{
	if (region1.getSizePix() == 0 || region2.getSizePix() == 0)
	{
		return false;
	}
	float intensitydiff = std::abs(region1.getMeanIntensity() - region2.getMeanIntensity()); // favs
	return (intensitydiff <= (threshold * 1.5f));
}

bool isSimilarIntensity3c(const Mat<uint8_t> &bgrImage,
						  const std::shared_ptr<Pixl> &current,
						  const std::shared_ptr<Pixl> &neighbor, float threshold)
{
	// TODO Test
	Vec<uint8_t> currentintensity = getPixlColor(current, bgrImage);
	Vec<uint8_t> neighborintensity = getPixlColor(neighbor, bgrImage);

	float intensitydiff = 0;
	Vec<float> intdif = (currentintensity.template convertDt<float>() - neighborintensity.template convertDt<float>());
	intdif.x = (intdif.x < 0) ? -intdif.x : intdif.x;
	intdif.y = (intdif.y < 0) ? -intdif.y : intdif.y;
	intdif.z = (intdif.z < 0) ? -intdif.z : intdif.z;

	intensitydiff = intdif.sumComponents() / 3;

	return (intensitydiff <= (threshold));
}

bool isSimilarIntensity3cGlobal(const Mat<uint8_t> &bgrImage,
								const Region &region,
								const std::shared_ptr<Pixl> &neighbor, float threshold)
{
	Vec<uint8_t> neighborintensity = bgrImage.atVec(neighbor->y, neighbor->x);
	// Différence d'intensité entre voisin et pixel en prenant en compte les trois couleurs
	float totalintensity = neighborintensity.sumComponents() / 3;
	return (std::abs((totalintensity)-region.getMeanIntensity())) < threshold;
}

bool isSimilarEucDist(const Mat<uint8_t> &bgrImage,
					  const std::shared_ptr<Pixl> &current,
					  const std::shared_ptr<Pixl> &neighbor, float threshold)
{
	// TODO Test
	Vec<uint8_t> tmpa = bgrImage.atVec(current->y, current->x);
	Vec<uint8_t> tmpb = bgrImage.atVec(current->y, current->x);

	Vec<uint8_t> eucldist = tmpa - tmpb;
	// Compare the intensity difference with the threshold ou

	return (Magn(eucldist) <= (threshold));
}

// TODO tweak the formula
bool isSimilarLuminance3cGlobal(const Mat<uint8_t> &bgrImage,
								const Region &region,
								const std::shared_ptr<Pixl> &neighbor,
								float threshold)
{
	Vec<float> neighborIntensity = (bgrImage.atVec(neighbor->y, neighbor->x)).template convertDt<float>();
	float weightedIntensity = 0;
	// Calculate luminance-weighted intensity BGR
	if (bgrImage.getClrspace() == PixlColorSpace::RGB)
	{
		weightedIntensity = 0.299 * neighborIntensity.x + 0.587 * neighborIntensity.y + 0.114 * neighborIntensity.z;
	}
	else if (bgrImage.getClrspace() == PixlColorSpace::BGR)
	{
		weightedIntensity = 0.299 * neighborIntensity.z + 0.587 * neighborIntensity.y + 0.114 * neighborIntensity.x;
	}
	else
	{
		return false;
	}
	// Compare the intensity difference with the threshold
	return std::abs(weightedIntensity - region.getMeanIntensity()) < threshold;
}

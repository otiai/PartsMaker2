#pragma once

#include "ImageRect.h"
#include "ClosedRegion.h"
#include <vector>

typedef unsigned int RegionID;
typedef ImageRect<RegionID> IDMap;
typedef float Real;

class AnimeFrame
{
public:
	AnimeFrame();
	~AnimeFrame();

public:
	bool loadInputImage(const char* filePath);

	inline int getNumRegions() const { return (int)m_Regions.size(); }

	const std::vector<ClosedRegion*> &getRegions() const { return m_Regions; }
	std::vector<ClosedRegion*> &getRegions() { return m_Regions; }

	
	ClosedRegion* getRegion(int ri) { return m_Regions[ri]; }

	const IDMap &getIDMap() const { return m_IDMap; }
	IDMap &getIDMap() { return m_IDMap; }

	const ImageRGBu &getColorImage() const { return m_ColorImage; } 
	ImageRGBu &getColorImage() { return m_ColorImage; }

private:
	bool initialize();
	bool finalize();

private:
	std::vector<ClosedRegion*> m_Regions;
	IDMap m_IDMap;
	ImageRGBu m_ColorImage;
};

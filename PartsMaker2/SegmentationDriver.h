#include "ivec.h"
#include "ImageRect.h"
#include "AnimeFrame.h"

class SegmentationDriver
{
public:
	bool applySegmentation( AnimeFrame &frame );
	void dumpIDMaps( const AnimeFrame &frame, const char *idMapFilename );
	void traceRegionBoundaries( AnimeFrame &frame );
	void buildRegionMap( AnimeFrame& frame );
	void colorFloodFill( int xSeed, int ySeed, const ImageRGBu &colorImage, ImageRect<RegionID> &idMap, RegionID id );

private:
	bool isBoundary( RegionID id, int xi, int yi, const IDMap &idMap ) const;
};
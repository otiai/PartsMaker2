#include "SegmentationDriver.h"
#include <stack>
#include "OpenCVImageIO.h"
#include "Config.h"
#include <QGLWidget>
using namespace std;
using namespace MyAlgebra;
using namespace IntVec;


bool SegmentationDriver::applySegmentation( AnimeFrame &frame )
{
	IDMap &idMap = frame.getIDMap();
	ImageRGBu &colorImage = frame.getColorImage();
	
	int w = colorImage.getWidth();
	int h = colorImage.getHeight();
	
	idMap.allocate( w, h );
	idMap.fill( Config::FalseRegionID );

	vector<ClosedRegion*> &regions = frame.getRegions();
	regions.clear();

	int currentID = 0;

	for (int yi=0; yi<h; yi++)
	{
		
		for (int xi=0; xi<w; xi++)
		{
			if (idMap(xi,yi) == Config::FalseRegionID)
			{
				if(colorImage(xi, yi) == Config::BackColor)
				{
					colorFloodFill(xi, yi, colorImage, idMap, Config::BackRegionID);
				}
				else
				{
					colorFloodFill(xi, yi, colorImage, idMap, currentID);
					const ubvec3 regionColor = colorImage(xi, yi);
					ClosedRegion* r = new ClosedRegion();
					r->setID( currentID );
					r->setRegionColor(regionColor);
					regions.push_back( r );

					currentID++;
				}
			}
		}
	}

	buildRegionMap( frame );

	return true;
}

void SegmentationDriver::colorFloodFill( int xSeed, int ySeed, const ImageRGBu &colorImage, ImageRect<RegionID> &idMap, RegionID id )
{
	const int w = colorImage.getWidth();
	const int h = colorImage.getHeight();

	const ubvec3 seedColor = colorImage(xSeed, ySeed);

	idMap(xSeed, ySeed) = id;

	stack<ivec2> traversalStack;
	traversalStack.push( ivec2(xSeed,ySeed) );

	while ( ! traversalStack.empty() )
	{
		const ivec2 pi = traversalStack.top();
		traversalStack.pop();

		const int x = pi.x;
		const int y = pi.y;

		// scan left
		int xLeft = x;
		while ( xLeft>=0 && seedColor == colorImage(xLeft,y))
		{
			idMap(xLeft,y) = id;
			xLeft--;
		}
		xLeft++;

		// scan right
		int xRight = x+1;
		while ( xRight<w && seedColor == colorImage(xRight,y))
		{
			idMap(xRight,y) = id;
			xRight++;
		}
		xRight--;

		for (int xi=xLeft; xi<=xRight; xi++)
		{
			if ( y > 0 )
				if ( seedColor != colorImage(xi+1,y-1) && seedColor == colorImage(xi,y-1) && idMap(xi,y-1)!=id )
					traversalStack.push( ivec2(xi,y-1) );

			if ( y < h-1 )
				if ( seedColor != colorImage(xi+1,y+1) && seedColor == colorImage(xi,y+1) && idMap(xi,y+1)!=id )
					traversalStack.push( ivec2(xi,y+1) );
		}

		if ( y > 0 )
			if ( seedColor == colorImage(xRight,y-1) && idMap(xRight,y-1)!=id )
				traversalStack.push( ivec2(xRight,y-1) );

		if ( y < h-1 )
			if ( seedColor == colorImage(xRight,y+1) && idMap(xRight,y+1)!=id )
				traversalStack.push( ivec2(xRight,y+1) );
	}
}

void SegmentationDriver::traceRegionBoundaries( AnimeFrame &frame )
{
	const IDMap &idMap = frame.getIDMap();
	ImageRGBu &colorImage = frame.getColorImage();
	
	
	if ( ! idMap.getData())
	{
		cerr << __FUNCTION__ << ": id map not ready" << endl;
		return;
	}

	const int w = colorImage.getHeight();
	const int h = colorImage.getWidth();

	vector<ClosedRegion*> &regions = frame.getRegions();
	const int nRegions = (int)regions.size();

	// 領域の境界のピクセルをクリアしておく
	for (int ri=0; ri<nRegions; ri++)
		regions[ri]->getBoundaryPixels().clear();

	ImageRect<bool> visitedMap(w,h);
	visitedMap.fill( false );

	for (int yi=0; yi<h; yi++)
	{
		for (int xi=0; xi<w; xi++)
		{
			const int id = idMap(xi,yi);

			if ( 0<=id && id<nRegions && isBoundary(id,xi,yi,idMap) && ! visitedMap(xi,yi) ) // そのピクセルが境界のピクセルなら
			{
				ClosedRegion* r = regions[id];
				vector<ivec2> &boundaryPixels = r->getBoundaryPixels();

				int xj = xi;
				int yj = yi;

				do {
					//cerr << "boundary (" << xj << "," << yj << ")" << endl;

					visitedMap(xj,yj) = true;
					boundaryPixels.push_back( ivec2(xj,yj) );

					if ( boundaryPixels.size() >= 100000 )
					{
						cerr << __FUNCTION__ << ": too many boundary pixels" << endl;
						break;
					}

					if (xj>0 && yj>0 && isBoundary(id,xj-1,yj-1,idMap) && !visitedMap(xj-1,yj-1))
					{
						xj--;
						yj--;
					}
					else if (yj>0 && isBoundary(id,xj,yj-1,idMap) && !visitedMap(xj,yj-1))
					{
						yj--;
					}
					else if (xj<w-1 && yj>0 && isBoundary(id,xj+1,yj-1,idMap) && !visitedMap(xj+1,yj-1))
					{
						xj++;
						yj--;
					}
					else if (xj<w-1 && isBoundary(id,xj+1,yj,idMap) && !visitedMap(xj+1,yj))
					{
						xj++;
					}
					else if (xj<w-1 && yj<h-1 && isBoundary(id,xj+1,yj+1,idMap) && !visitedMap(xj+1,yj+1))
					{
						xj++;
						yj++;
					}
					else if (yj<h-1 && isBoundary(id,xj,yj+1,idMap) && !visitedMap(xj,yj+1))
					{
						yj++;
					}
					else if (xj>0 && yj<h-1 && isBoundary(id,xj-1,yj+1,idMap) && !visitedMap(xj-1,yj+1))
					{
						xj--;
						yj++;
					}
					else if (xj>0 && isBoundary(id,xj-1,yj,idMap) && !visitedMap(xj-1,yj))
					{
						xj--;
					}
					else
					{
						break;	// なにも動かなかった
					}
				} while ( ! (xj==xi && yj==yi) );
			}
		}
	}

	// バウンディングボックスの計算もここで行う
	for (int ri=0; ri<nRegions; ri++)
	{
		ClosedRegion* r = regions[ri];
		r->setBboxMin( w, h );
		r->setBboxMax( 0, 0 );

		const vector<ivec2> &boundary = r->getBoundaryPixels();
		
		for (int bi=0; bi<(int)boundary.size(); bi++)
		{
			const ivec2 p = boundary[bi];
			const ivec2 bMin = r->getBboxMin();
			const ivec2 bMax = r->getBboxMax();

			r->setBboxMin( min(bMin.x,p.x), min(bMin.y,p.y) );
			r->setBboxMax( max(bMax.x,p.x), max(bMax.y,p.y) );
		}
	}
}

bool SegmentationDriver::isBoundary( RegionID id, int xi, int yi, const IDMap &idMap ) const
{
	const int w = idMap.getWidth();
	const int h = idMap.getHeight();

	const RegionID newID = idMap(xi,yi);

	return (id == newID) && (xi==0 || xi==w-1 || yi==0 || yi==h-1 || 
		id != idMap(xi-1,yi) || id != idMap(xi,yi-1) || id != idMap(xi+1,yi) || id != idMap(xi,yi+1));
}

void SegmentationDriver::dumpIDMaps( const AnimeFrame &frame, const char *idMapFilename)
{
	const IDMap &idMap = frame.getIDMap();
	
	const int w = idMap.getWidth();
	const int h = idMap.getHeight();

	ImageRGBu tmpImg(w,h), extImg(w,h);
	const int nRegions = frame.getNumRegions();

	tmpImg.fill( ubvec3(0,0,0) );

	vector<ubvec3> cols;
	for (int ri=0; ri<nRegions; ri++)
		cols.push_back( ubvec3(rand()%256, rand()%256, rand()%256) );

	for (int yi=0; yi<h; yi++)
	{
		for (int xi=0; xi<w; xi++)
		{
			const int id = idMap(xi,yi);

			if (id >= 0 && id < nRegions)
				tmpImg(xi,yi) = cols[id];
		}
	}

	OpenCVImageIO io;
	io.save(idMapFilename, tmpImg);
}

void SegmentationDriver::buildRegionMap( AnimeFrame& frame )
{
	const IDMap &idMap = frame.getIDMap();
	
	if ( ! idMap.getData())
	{
		cerr << __FUNCTION__ << ": id map not ready" << endl;
		return;
	}

	const int w = idMap.getWidth();
	const int h = idMap.getHeight();
	int nRegions = frame.getNumRegions();

	for(int i = 0; i < nRegions; i++)
	{
		ClosedRegion* r = frame.getRegion(i);
		RegionMap& regionMap = r->getRegionMap();
		regionMap.allocate( w, h );
		regionMap.fill( IntVec::ubvec4(0,0,0,0) );

		for(int y = 0; y < h; y++)
		{
			for(int x = 0; x < w; x++)
			{
				int regionID = r->getID();
				int id = idMap(x, y);
				if(regionID == id)
				{
					ImageRGBu &colorImage = frame.getColorImage();
					IntVec::ubvec3 color = colorImage(x, y);
					regionMap(x, y) = IntVec::ubvec4(color.r, color.g, color.b, 255);
				}
			}
		}
#if 0 // デバッグ用
		char str[256];
		sprintf(str, "ResultImage/region_%d.png", r->getID()); 

		OpenCVImageIO io;
		io.save(str, regionMap);
#endif
	}
}

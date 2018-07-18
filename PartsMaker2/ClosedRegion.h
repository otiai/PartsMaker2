#pragma once

#include "my_algebra.h"
#include "ivec.h"
#include "ImageRect.h"
#include <vector>
#include <Qvector>
#include <QVector2D>
#include <QVector3D>
#include <QImage>

typedef ImageRGBAu RegionMap;

class Line
{
public:
	QVector<QVector2D>& getPoints(){ return points_; }
	void setPoints(QVector<QVector2D>& points);
private:
	QVector<QVector2D>	points_;
};

class RegionLinkData;
class ClosedRegion
{
public:
	ClosedRegion();
	~ClosedRegion();

	inline void setID(int id) { m_ID = id; }
	inline int getID() const { return m_ID; }

	void setNumPixels(int n) { m_NumPixels = n; }
	int getNumPixels() const { return m_NumPixels; }

	const IntVec::ivec2 &getBboxMin() const { return m_BboxMin; }
	IntVec::ivec2 &getBboxMin() { return m_BboxMin; }

	void setBboxMin(int x, int y) { m_BboxMin.set(x,y); }
	void setBboxMin(const IntVec::ivec2& i) { m_BboxMin = i; }

	const IntVec::ivec2 &getBboxMax() const { return m_BboxMax; }
	IntVec::ivec2 &getBboxMax() { return m_BboxMax; }

	void setBboxMax(int x, int y) { m_BboxMax.set(x,y); }
	void setBboxMax(const IntVec::ivec2& i) { m_BboxMax = i; }

	const std::vector<IntVec::ivec2> &getBoundaryPixels() const { return m_BoundaryPixels; }
	std::vector<IntVec::ivec2> &getBoundaryPixels() { return m_BoundaryPixels; }
	void setBoundaryStartPoint(int index);

	QVector<Line*>* getBoundaryLines(){ return &m_Lines; }

	void resetFeaturePoint();
	void addFeaturePoint(int insertIndex, float mappedRatio);

	//
	const QVector<int>* getFeaturePointIndices(){ return &m_FeaturePointIndices; }
	void setFeaturePoint(int dataIndex, int pixelIndex);
	int addFeaturePoint(int pixelIndex);

	const IntVec::ubvec3 getRegionColor() const { return m_RegionColor; }
	void setRegionColor(IntVec::ubvec3 color){ m_RegionColor = color; }

	void drawBoundary() const;
	
	RegionMap &getRegionMap(){ return *m_RegionMap; }

	void fillHoles();
	bool modifyRegion(ImageRGBAu& scribbleBuffer);
	bool replaceRegionMap(QImage& replaceMap);

	void traceRegionBoundaries();
	void serializeRegionBoundaries();
	void createLines();

	void setRegionLinkData(RegionLinkData* data){ m_RegionLinkDataPtr = data; }
	RegionLinkData* getRegionLinkData(){ return m_RegionLinkDataPtr; }

	void setPos3D(QVector3D p){ m_Pos3D = p; }
	QVector3D getPos3D(){ return m_Pos3D; }

public:
	static float calcSimirarity_Ours(ClosedRegion& a, ClosedRegion& b);
	static float calcSimirarity(ClosedRegion& a, ClosedRegion& b);
	static float calcSmallestEuclideanDistance(ClosedRegion& a, ClosedRegion& b);
	static int calcOverlapSize(ClosedRegion& a, ClosedRegion& b);
	static int calcSize(ClosedRegion& r);
	static float calcPerimeter(ClosedRegion& r);

private:
	void floodFill();
	bool isBoundary( IntVec::ubvec4 color, int xi, int yi, const RegionMap &regionMap ) const;

private:
	int m_ID;
	int m_NumPixels;
	IntVec::ivec2				m_BboxMin, m_BboxMax;
	std::vector<IntVec::ivec2>	m_BoundaryPixels;
	RegionMap*					m_RegionMap;
	IntVec::ubvec3				m_RegionColor;
	RegionLinkData*				m_RegionLinkDataPtr; // 対応データのポインタ

	QVector<Line*>				m_Lines;
	QVector<int>				m_FeaturePointIndices;
	QVector3D					m_Pos3D;
};

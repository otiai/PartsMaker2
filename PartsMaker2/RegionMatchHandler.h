#ifndef REGION_MATCH_HANDLER_H
#define REGION_MATCH_HANDLER_H

#include <QVector>
#include <QColor>
#include <QVector3D>
#include <QVector2D>
#include <QRect>

#define COLOR_MAX 64


// 拡張するためにVIEWで定義
// 現在（2014/05/07はVIEW_FRONTがsrc、VIEW_SIDE_RIGHTがdstとして扱う）
enum VIEW
{
	VIEW_MAIN = 0,
	VIEW_FRONT = VIEW_MAIN,
	VIEW_SIDE_RIGHT,

	VIEW_MAX,
};

// 領域の対応関係を保持するためのデータ構造
// メインビューの各領域に対応する領域が記述される
// このため、メインビューは全ての領域を含まないといけない
// メインビューに存在しない領域（出現領域）を記述するためには、ダミーの領域をユーザが作る必要がある

class QMatrix4x4;
class ClosedRegion;
class RegionLinkData
{
public:
	RegionLinkData(){ init(); }
	~RegionLinkData(){}

	void init();
	void clear();

	void setData(ClosedRegion* r, int viewID);
	ClosedRegion* getRegion(int viewID);
	int getUniqueID(){ return uniqueID; }

	void resetFeaturePoint();
	void createLines();
	void calcDepth();
	QRect calcBoundaryPixels(float rotX, float rotY);
	const QVector3D& get3DPos(){ return pos3D_; }

	const QVector<QVector2D>* getBoundaryPixels(){ return &boundaryPixels_; }

	static void resetCreateDataNum(){ createdDataNum = 0;}
private:
	void calc3DLine(ClosedRegion* r, QMatrix4x4* matProjInv, QMatrix4x4* matViewInv, QVector3D* outStart, QVector3D* outEnd);

private:
	ClosedRegion*		regions[VIEW_MAX];
	int					uniqueID; // カラーインデックスを取得するためにユニークＩＤ作成
	static int			createdDataNum;
	QVector3D			pos3D_;
	QVector<QVector2D>	boundaryPixels_;
};

class RegionLinkDataManager
{
public:
	RegionLinkDataManager();
	~RegionLinkDataManager();

	void createData(ClosedRegion* r);
	void deleteData(RegionLinkData* linkData);
	void deleteAll();
	void link(ClosedRegion* mainRegion, ClosedRegion* subRegion, int viewID);
	void clearLink(ClosedRegion* region);
	void reCalcDepth();

	void debugPrint();

	QColor& getIndexColor(ClosedRegion* r);
	QColor& getIndexColor(int dataIndex);

	const QVector<RegionLinkData*>* getDatas(){ return &regionLinkDatas_; };

private:
	QVector<RegionLinkData*>	regionLinkDatas_;
	QColor						colorTable_[COLOR_MAX];
	QColor						noneColor_;
};
#endif // REGION_MATCH_HANDLER_H
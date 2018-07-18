#ifndef OBJECT_MANAGER_H
#define OBJECT_MANAGER_H

#include "ImageRect.h"
#include <QVector>
#include <QRect>
#include <QVector3D>
#include "RegionMatchHandler.h"
#include <QPoint>
#include <QMatrix4x4>
#include <QVector2D>
#include <QString>

static QColor sLinkingLineColor = Qt::yellow;
static QColor sSelectRegionEdgeColor = Qt::yellow;
static QColor sSelectEnableRegionEdgeColor = Qt::green;

class MainWindow;
class RegionLinkData;
struct SelectRegionData
{
public:
	SelectRegionData()
	{
		clearAll();
	}

	void clearAll()
	{
		viewID = -1;
		selectEnableLinkData = NULL;
		selectLinkData = NULL;
		selectRegion = NULL;
		selectFeatureIndex = -1;
		selectedViewID = -1;
	};

	int viewID;			// カーソルがある方のビューID
	int selectedViewID; // ユーザが選択したビューID（クリックしたほうのビュー）
	RegionLinkData*	selectEnableLinkData;
	RegionLinkData* selectLinkData;
	ClosedRegion*	selectRegion;
	ClosedRegion*	selectEnableRegion;
	int				selectFeatureIndex;	// 選択特徴点データのインデックス
	
};

enum EditMode
{
	MODE_REGION_MATCH,
	MODE_FEATUREPOINT_EDIT,

	MODE_
};

class AnimeFrame;
class ObjectManager
{
public:
	static ObjectManager* create();
	static void destroy();
	static ObjectManager* getInstance(){ return instance_; }
	
	void initialize(MainWindow* w);
	void finalize();

	AnimeFrame* getSrcFrame(){ return srcFrame_; }
	AnimeFrame* getDstFrame(){ return dstFrame_; }

	SelectRegionData* getSelectRegionData(){ return &selectRegionData_; }

	RegionLinkDataManager* getRegionLinkDataManager(){ return &regionLinkDataManager_; }

	void setEditMode(int m){ editMode_ = m;}
	int getEditMode(){ return editMode_; }

	void linkDataUpdated();
	bool createMatchedRegion(ClosedRegion* r, int selfViewID);
	bool deleteClosedRegion(ClosedRegion* r, int viewID);
	
	void setRegionLinking(bool linking){ isRegionLinking_ = linking;}
	bool isRegionLinking(){ return isRegionLinking_; }
	void setDragStartGlobalPos(QPoint p){ dragStartGlobalPos_ = p;}
	const QPoint getDragStartGlobalPos(){ return dragStartGlobalPos_; }
	void setDragEndGlobalPos(QPoint p){ dragEndGlobalPos_ = p; }
	const QPoint getDragEndGlobalPos(){ return dragEndGlobalPos_; }

	bool combineRegion(ClosedRegion& r, ImageRGBAu& scribbleBuffer, int viewID);
	bool divideRegion(ClosedRegion& r, ImageRGBAu& scribbleBuffer, int viewID);

	static void setCurrentSrcRegionID(int id){ currentSrcRegionID_ = id; }
	static void setCurrentDstRegionID(int id){ currentDstRegionID_ = id; }

	void setSrcRotation(QVector2D rot);
	void setDstRotation(QVector2D rot);
	const QVector2D& getSrcRotation(){ return srcRot_; }
	const QVector2D& getDstRotation(){ return dstRot_; }
	const QMatrix4x4& getSrcPoseMatrix(){ return srcPoseMatrix_; }
	const QMatrix4x4& getDstPoseMatrix(){ return dstPoseMatrix_; }

	void loadImageFiles();

	void setSrcImageFileName(QString s){ srcImageFileName_ = s; }
	void setDstImageFileName(QString s){ dstImageFileName_ = s; }

	void reCalcDepth();


	void changeEdgeWidth(int w);
	int getEdgeWidth(){ return edgeWidth_; }

private:
	void deleteResultDatas();
	void regionMatching();
	
	
private:
	static ObjectManager*	instance_;
	AnimeFrame*				srcFrame_;
	AnimeFrame*				dstFrame_;
	SelectRegionData		selectRegionData_;
	RegionLinkDataManager	regionLinkDataManager_;
	int						editMode_;
	MainWindow*				refMainWindow_;

	bool					isRegionLinking_;
	QPoint					dragStartGlobalPos_;
	QPoint					dragEndGlobalPos_;

	static int				currentSrcRegionID_;
	static int				currentDstRegionID_;

	QVector2D				srcRot_;
	QVector2D				dstRot_;
	QMatrix4x4				srcPoseMatrix_;
	QMatrix4x4				dstPoseMatrix_;

	QString					srcImageFileName_;
	QString					dstImageFileName_;

	int						edgeWidth_;
};

#endif // OBJECT_MANAGER_H
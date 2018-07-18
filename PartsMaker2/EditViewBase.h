#ifndef EDIT_VIEW_BASE_H
#define EDIT_VIEW_BASE_H

#include <QGLWidget>
#include <QRect>
#include "ClosedRegion.h"
#include <QColor>
#include "RegionMatchHandler.h"

class AnimeFrame;


class EditViewBase : public QGLWidget
{
	Q_OBJECT
signals:
	void closed();
	void regionModified();
	void regionSelectEnabled();
	void regionSelected();
	void matchedRegionCreated();
	void globalPosUpdated();

public slots:
	void rebuildTextures();
	void rebuildPolygons();
	
protected:
	class RegionTextureData
	{
	public:
		RegionTextureData()
		{
			regionID = -1;
			textureID = -1;
			regionMapMono = NULL;
			regionPtr = NULL;
		};
	public:
		int				regionID;
		unsigned int	textureID;
		QRectF			boundingBox;
		RegionMap*		regionMapMono;
		QColor			regionColor;
		ClosedRegion*   regionPtr;
	};

public:
	EditViewBase(QWidget *parent = 0);
	~EditViewBase();
	void initImage();

protected:
	void initializeGL();
	void resizeGL(int width, int height);
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent * event);
	void keyPressEvent(QKeyEvent *event);
	void closeEvent(QCloseEvent* event);

	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dragLeaveEvent(QDragLeaveEvent *event);

	void setupViewport(int width, int height);
	void draw2D(QPainter* painter);

	void makeTextures();
	void deleteTextures();

	virtual AnimeFrame* getAnimeFrame() = 0;
	virtual int getViewID() = 0;
	
	void drawRegion();
	void drawBoundingBox();
	void drawBoundaryLine();
	void drawFeaturePoints(QPainter* painter);
	void drawRegionPolygon(QPainter& painter, QPolygon& polygon, QColor& color, QColor& edgeColor, 
		QPoint& pos, bool isEdgeDraw, QPointF scale = QPointF(1.0f, 1.0f));


	void setRegionColor(int index);
	void hitTest();
	void hitTest_Vertices();
	void hitTest_FeaturePoints();
	void movedFeaturePoint();
	void startDrag();
	void setFeaturePoint();
	void createMatchedRegion();
	QPoint makePolygon(ClosedRegion& r, QColor& outColor, QPolygon& outPolygon);
	void clearPolygon();
	
	void updateSelect();
	void updateSelectEnable();

	int getDataIndex(int regionID);

protected:
	QPoint					mousePos_;
	QPoint					startPos_;
	QVector<RegionTextureData>	textureDatas_;
	float					dispScale_;

	bool					isDispMatchColor_;
	int						selectRegionID_;
	int						selectEnableRegionID_;

	int						selectPointIndex_;
	int						moveEnableFeaturePointIndex_;
	bool					isMoveFeaturePoint_;

	QPolygon				selectPolygon_;
	QColor					selectPolygonColor_;
	QPoint					selectPolygonPos_;

	QPolygon				selectEnablePolygon_;
	QColor					selectEnablePolygonColor_;
	QPoint					selectEnablePolygonPos_;
};


#endif // EDIT_VIEW_BASE_H
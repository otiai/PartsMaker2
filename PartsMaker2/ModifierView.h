#ifndef MODIFIER_VIEW_H
#define MODIFIER_VIEW_H

#include <QGLWidget>
#include "ScribbleBrush.h"
#include "ImageRect.h"
#include <QRect>
#include "ClosedRegion.h"
#include <QPolygon>
#include <QColor>

class QImage;
class ClosedRegion;
class ModifierView : public QGLWidget
{
	Q_OBJECT

signals:
	void closed();
	void regionModified();
	void regionDeleted();
	void regionSelected();
	void regionDivided();

public slots:
	void rebuild();
	void setFreeHandMode();
	void setTranslationMode();
	void setDivideMode();
	void setCombineMode();
	void setSelectMode();

protected:
	class RegionTextureData
	{
	public:
		RegionTextureData()
		{
			regionID = -1;
			textureID = -1;
			regionPtr = NULL;
		};
	public:
		int				regionID;
		unsigned int	textureID;
		ClosedRegion*   regionPtr;
		QRectF			boundingBox;
	};


	enum State
	{
		STATE_SELECT_NORMAL,

		STATE_TRANSLATION_NORMAL,
		STATE_TRANSLATION_SELECTED,

		STATE_FREEHAND_NORMAL,
		STATE_DIVIDE_NORMAL,
		STATE_COMBINE_NORMAL,

		STATE_,
	};

public:
	ModifierView(QWidget *parent = 0);
	~ModifierView();

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
	
	void setupViewport(int width, int height);
	void draw2D(QPainter* painter);

	void makeTextures();
	void updateTextures();
	void deleteTextures();

	void drawRegion();
	void modifyRegion();
	void combineRegion();
	void translateRegion();
	void divideRegion();

	// scribble
	void createScribbleImage();
	void updateScribbleImage();
	void deleteScribbleImage();
	void clearScribble();

	void setState(int s){ state_ = s; }
	int getState(){ return state_; }

	void makePolygon();
	void makePolygonTexture();
	void movePolygon();
	void drawRegionPolygon(QPainter& painter, QPolygon& polygon, QColor& color, QColor& edgeColor, 
		QPoint& pos, bool isEdgeDraw, QPointF scale = QPointF(1.0f, 1.0f));

	void makeSelectEnablePolygon();
	QPoint makePolygon(ClosedRegion& r, QColor& outColor, QPolygon& outPolygon);

	void hitTest();
	int getRegionIndex(int regionID);
	void selectRegion();

protected:
	QPoint					mousePos_;
	QPoint					mousePosOld_;

	float					dispScale_;
	ClosedRegion*			selectRegion_;
	QVector<RegionTextureData>	textureDatas_;

	ScribbleBrush			scribbleBrush_;
	ImageRGBAu				scribbleBuffer_;
	QImage*					scribbleBufferImage_;
	bool					isDispOtherRegions_;

	int						state_;
	QPolygon				regionPolygon_;
	QColor					polygonColor_;
	QPoint					polygonPos_;
	QImage*					offscreenImage_;
	int						selectRegionTextureID_;
	QPointF					polygonScale_;
	
	int						selectEnableRegionID_;
	QPolygon				selectEnablePolygon_;
	QColor					selectEnablePolygonColor_;
	QPoint					selectEnablePolygonPos_;
};
#endif // MODIFIER_VIEW_H
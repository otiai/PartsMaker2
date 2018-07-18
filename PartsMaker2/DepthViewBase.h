#ifndef DEPTH_VIEW_BASE_H
#define DEPTH_VIEW_BASE_H

#include <QGLWidget>
#include "ImageRect.h"
#include <QVector3D>
#include <QVector>


class QImage;
class RegionLinkData;
class DispData
{
public:
	DispData();
	~DispData();
public:
	GLuint		textureID;
	QImage*	image;
	RegionLinkData* linkData;
	QRect			bBox;
	QColor			regionColor;
};


class DepthViewBase : public QGLWidget
{
	Q_OBJECT
signals:
	void closed();

public slots:
	void rebuild();

public:
	DepthViewBase(QWidget *parent = 0);
	~DepthViewBase();
	
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
	void deleteTextures();

	void saveImage();

protected:
	QPoint					mousePos_;
	QPoint					mouseStartPos_;
	bool					isDrag_;
	bool					isBillboard_;
	QVector3D				rot_;
	float					dispScale_;

	QVector<DispData*>		dispDatas_;
};


#endif // DEPTH_VIEW_BASE_H
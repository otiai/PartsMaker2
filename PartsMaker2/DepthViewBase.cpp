#include "DepthViewBase.h"
#include <QtGui>
#include "AnimeFrame.h"
#include "ClosedRegion.h"
#include <vector>
#include <gl/glu.h>
#include "ObjectManager.h"
#include "Utility.h"

#define DISP_BOUNDING_BOX 0
#define DISP_EYE_DIRECTION 0 // 視線確認用の線を表示
static float sWeight = 0.4f; // マウス回転の重み
static float sRotValue = 5.0f;


DispData::DispData()
{
	image = NULL;
	linkData = NULL;
}

DispData::~DispData()
{
	if(image)
	{
		delete image;
	}
	image = NULL;

	linkData = NULL;
}


//--------------------------------------------------
/*!
	@brief	コンストラクタ
*/
DepthViewBase::DepthViewBase(QWidget *parent)
{
	// OpenGLを使うときはfalseにする
	setAutoFillBackground(false);

	// マウスボタンを押してなくてもMouseMoveイベントが来るようになる
	setMouseTracking(true);

	// デフォルトウィンドウサイズ設定
	resize(500, 500);
	setMinimumSize( width(), height() );
	setMaximumSize( width(), height() );

	isDrag_ = false;

	// キーボードにフォーカスする
	setFocusPolicy(Qt::ClickFocus);

	isBillboard_ = true;

	rot_ = QVector3D(0,0,0);

	dispScale_ = 1.0f;
}

//--------------------------------------------------
/*!
	@brief	デストラクタ
*/
DepthViewBase::~DepthViewBase()
{
	deleteTextures();
}

//--------------------------------------------------
/*!
	@brief	初期化
*/
void DepthViewBase::initializeGL()
{
	AnimeFrame* frame = ObjectManager::getInstance()->getSrcFrame();
	if(frame)
	{
		initImage();
	}
}

//--------------------------------------------------
/*!
	@brief	ファイル読み込み後の初期化
*/
void DepthViewBase::initImage()
{
	AnimeFrame* frame = ObjectManager::getInstance()->getSrcFrame();
	int w = frame->getIDMap().getWidth();
	int h = frame->getIDMap().getHeight();
	setMaximumSize( w, h );
	setMinimumSize( w, h );
	
	resize(w, h);
	rebuild();
}

//--------------------------------------------------
/*!
	@brief	ビューポート設定
*/
void DepthViewBase::setupViewport(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, +1.0, -1.0, 1.0, 0.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
}
//--------------------------------------------------
/*!
	@brief	リサイズ
*/
void DepthViewBase::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);
	glLoadIdentity();
}

//--------------------------------------------------
/*!
	@brief	描画
*/
void DepthViewBase::paintEvent(QPaintEvent *event)
{
	// ここでのOpenGLの描画を可能にする
	makeCurrent();

	// クリア
	glEnable(GL_DEPTH_TEST);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glTranslated(0,0,-10);
	glRotated(rot_.y(), 0, 1, 0);
	glRotated(rot_.x(), 1, 0, 0);
	

	// OpenGLでの描画
	glScalef(dispScale_, dispScale_, dispScale_);

	// テクスチャ描画
	makeTextures();

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.5);

	for(int i = 0; i < dispDatas_.size(); i++)
	{
		DispData* data = dispDatas_.at(i);

		QRect bBox = data->bBox;
		ClosedRegion* front = data->linkData->getRegion(VIEW_FRONT);
		float w = front->getRegionMap().getWidth();
		float h = front->getRegionMap().getHeight();
		float fw = 2.0 * bBox.width() / w;
		float fh = 2.0 * bBox.height() / h;

		glPushMatrix();

		QMatrix4x4 matRot, matInv;
		Utility::getMatrix(matRot, GL_MODELVIEW_MATRIX);
		const QVector3D& pos = data->linkData->get3DPos();
		glTranslatef(pos.x(), pos.y(), pos.z());

		if(isBillboard_)
		{
			matRot.setColumn(3, QVector4D(0,0,0,1));
			matInv = matRot.inverted();
			Utility::multMatrix(matInv);
		}

		glColor3f(1.0, 1.0, 1.0);
		//glScalef(fw, fh, 1.0);
		glScalef(2.0, 2.0, 1.0);
		glBindTexture(GL_TEXTURE_2D, data->textureID);
		glBegin(GL_POLYGON);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(-0.5,  0.5, 0.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f( 0.5,  0.5, 0.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f( 0.5, -0.5, 0.0f);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(-0.5, -0.5, 0.0f);
		glEnd();
	
#if DISP_BOUNDING_BOX
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);

		glColor3f(1.0, 1.0, 1.0);
		glBegin(GL_LINE_LOOP);
			glVertex3f(-0.5,  0.5, 0.0f);
			glVertex3f( 0.5,  0.5, 0.0f);
			glVertex3f( 0.5, -0.5, 0.0f);
			glVertex3f(-0.5, -0.5, 0.0f);
		glEnd();

		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);
#endif // DISP_BOUNDING_BOX

		glPopMatrix();
		
	}

	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	
		
	setupViewport(width(), height());
	glDisable(GL_DEPTH_TEST);

	// QPainterによる描画（2D）
	QPainter painter(this);
	draw2D(&painter);
	painter.end();
}

//--------------------------------------------------
/*!
	@brief	QPainterによる描画（2D）
*/
void DepthViewBase::draw2D(QPainter* painter)
{
	QString str = QString("rotX = %1").arg(rot_.x());
	QString str2 = QString("rotY = %1").arg(rot_.y());
	painter->setPen(Qt::green);
	//painter->drawText( 30, 30, str );
	//painter->drawText( 30, 50, str2 );
}

//--------------------------------------------------
/*!
	@brief	マウス（押し）イベント
*/
void DepthViewBase::mousePressEvent(QMouseEvent *event)
{
	/*
		event->button() == Qt::LeftButton
		などで押されたボタン取得
		マウス座標はevent->x(), event->y()などで取得
	*/
	if(event->button() == Qt::LeftButton)
	{
		mouseStartPos_ = event->pos();
		isDrag_ = true;
	}
}

//--------------------------------------------------
/*!
	@brief	マウス（離し）イベント
*/
void DepthViewBase::mouseReleaseEvent(QMouseEvent *event)
{
	isDrag_ = false;
}

//--------------------------------------------------
/*!
	@brief	マウス移動イベント
*/
void DepthViewBase::mouseMoveEvent(QMouseEvent *event)
{
	mousePos_ = event->pos();

	if(isDrag_)
	{
		static int xdir,ydir;
		xdir = mousePos_.x() - mouseStartPos_.x(); //ドラッグ操作での移動量を取得
		ydir = mousePos_.y() - mouseStartPos_.y();

		float rotX, rotY;
		rotX = rot_.x();
		rotX += (double)ydir * sWeight;
		rot_.setX(rotX);
		rotY = rot_.y();
		rotY += (double)xdir * sWeight;
		rot_.setY(rotY);
       
		mouseStartPos_ = mousePos_;
	}

	// 再描画リクエストを通知
	update();
}

//------------------------------------------------------
/*!
	@brief	キーボードイベント
*/
void DepthViewBase::keyPressEvent(QKeyEvent *event)
{
	/*
		event->key() == Qt::Key_S
		などで押されたキーを取得
	*/
	if(event->key() == Qt::Key_B)
	{
		isBillboard_ = !isBillboard_;
		update();
	}

	if(event->key() == Qt::Key_Right)
	{
		rot_.setY(rot_.y() + sRotValue);
		update();
	}

	if(event->key() == Qt::Key_Left)
	{
		rot_.setY(rot_.y() - sRotValue);
		update();
	}

	if(event->key() == Qt::Key_Up)
	{
		rot_.setX(rot_.x() + sRotValue);
		update();
	}

	if(event->key() == Qt::Key_Down)
	{
		rot_.setX(rot_.x() - sRotValue);
		update();
	}

	if(event->key() == Qt::Key_P)
	{
		saveImage();
	}

	if(event->key() == Qt::Key_R)
	{
		rot_.setX(0.0);
		rot_.setY(0.0);
		update();
	}
}

void DepthViewBase::makeTextures()
{
	makeCurrent();

	glEnable(GL_TEXTURE_2D);

	int w = width();
	int h = height();

	int penWidth = ObjectManager::getInstance()->getEdgeWidth();
	QPen pen(Qt::black);
	pen.setWidth(penWidth);
	
	for(int i = 0; i < dispDatas_.size(); i++)
	{
		DispData* data = dispDatas_.at(i);
		data->bBox = data->linkData->calcBoundaryPixels(rot_.x(), rot_.y());
		const QVector<QVector2D>* boundaryPixels = data->linkData->getBoundaryPixels();
		QPolygon ply;

		int bw = data->bBox.width();
		int bh = data->bBox.height();
		for(int j = 0; j < boundaryPixels->size(); j++)
		{
			float x = (boundaryPixels->at(j).x()) * bw + w * 0.5f;
			float y = h - ((boundaryPixels->at(j).y()) * bh + h * 0.5f);
			ply.append(QPoint(x, y));
		}

		data->image->fill(QColor(0,0,0,0));
		QPainter painter(data->image);
		QColor color = data->regionColor;
		QBrush brush(color, Qt::SolidPattern);
		painter.setPen(pen);
		painter.setBrush(brush);
		painter.drawPolygon(ply);
		glBindTexture(GL_TEXTURE_2D, data->textureID );
		QImage img = QGLWidget::convertToGLFormat(*data->image);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width(), height(), GL_RGBA, GL_UNSIGNED_BYTE, img.bits() );
	}

	glDisable(GL_TEXTURE_2D);
}

void DepthViewBase::deleteTextures()
{
	makeCurrent();

	for(int i = 0; i < dispDatas_.size(); i++)
	{
		DispData* data = dispDatas_.at(i);
		delete data;
	}
	dispDatas_.clear();
}

void DepthViewBase::closeEvent(QCloseEvent *event)
{
	emit closed();
}

void DepthViewBase::rebuild()
{
	deleteTextures();

	makeCurrent();
	
	glEnable(GL_TEXTURE_2D);

	int w = width();
	int h = height();
	RegionLinkDataManager* linkMgr = ObjectManager::getInstance()->getRegionLinkDataManager();
	const QVector<RegionLinkData*>* datas = linkMgr->getDatas();
	for(int i = 0; i < datas->size(); i++)
	{
		ClosedRegion* front = datas->at(i)->getRegion(VIEW_FRONT);
		ClosedRegion* side = datas->at(i)->getRegion(VIEW_SIDE_RIGHT);
		if(!front || !side)
			continue;

		DispData* data = new DispData;
		data->image = new QImage(QSize(w, h), QImage::Format_ARGB32);
		data->image->fill(QColor(0,0,0,0));
		

		glGenTextures(1, &data->textureID);
		glBindTexture(GL_TEXTURE_2D, data->textureID );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		QImage img = QGLWidget::convertToGLFormat(*data->image);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width(), height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());

		data->linkData = datas->at(i);
		data->regionColor = QColor(front->getRegionColor().r, front->getRegionColor().g, front->getRegionColor().b);
		dispDatas_.append(data);
	}

	glDisable(GL_TEXTURE_2D);

	makeTextures();
	update();
}

void DepthViewBase::saveImage()
{
	char str[256];
	int x = rot_.x();
	int y = rot_.y();
	sprintf(str, "SnapShots/img_x_%d_y_%d.png", x, y);

	QImage img = QImage(this->size(), QImage::Format_ARGB32_Premultiplied);
    glReadPixels(0, 0, this->width(), this->height(), GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
    img = img.rgbSwapped();
    img = img.mirrored();
    img.save(str);

}

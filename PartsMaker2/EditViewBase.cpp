#include "EditViewBase.h"
#include <QtGui>
#include "Utility.h"
#include "AnimeFrame.h"
#include "ClosedRegion.h"
#include <QVector2D>
#include "ObjectManager.h"
#include <QMatrix4x4>

static const float RADIUS_SIZE = 10.0f;
static const float SELECT_POINT_SIZE = 10.0f; 
//--------------------------------------------------
/*!
	@brief	コンストラクタ
*/
EditViewBase::EditViewBase(QWidget *parent)
{
	// OpenGLを使うときはfalseにする
	setAutoFillBackground(false);

	// マウスボタンを押してなくてもMouseMoveイベントが来るようになる
	setMouseTracking(true);

	// キーボードにフォーカスする
	setFocusPolicy(Qt::ClickFocus);
	
	resize(500, 500);
	setMinimumSize( width(), height() );
	setMaximumSize( width(), height() );

	selectRegionID_ = -1;
	selectEnableRegionID_ = -1;

	dispScale_ = 1.0f;
	isDispMatchColor_ = true;
	selectPointIndex_ = -1;
	moveEnableFeaturePointIndex_ = -1;
	isMoveFeaturePoint_ = false;

	// ドラッグ＆ドロップを有効にする
	setAcceptDrops(true);
}

//--------------------------------------------------
/*!
	@brief	デストラクタ
*/
EditViewBase::~EditViewBase()
{
	deleteTextures();
}

//--------------------------------------------------
/*!
	@brief	初期化
*/
void EditViewBase::initializeGL()
{
	AnimeFrame* frame = getAnimeFrame();
	if(frame)
	{
		initImage();
	}
}
//--------------------------------------------------
/*!
	@brief	ファイル読み後の初期化
*/
void EditViewBase::initImage()
{
	// デフォルトウィンドウサイズ設定
	AnimeFrame* frame = getAnimeFrame();
	int w = frame->getIDMap().getWidth();
	int h = frame->getIDMap().getHeight();
	setMinimumSize( w, h );
	setMaximumSize( w, h );
	resize(w, h);
	makeTextures();
	clearPolygon();
	update();
}

//--------------------------------------------------
/*!
	@brief	ビューポート設定
*/
void EditViewBase::setupViewport(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, +1.0, -1.0, 1.0, 1.0, -1.0); // プロジェクション行列を単位行列にするため、この値をセット
	glMatrixMode(GL_MODELVIEW);
}
//--------------------------------------------------
/*!
	@brief	リサイズ
*/
void EditViewBase::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);
	glLoadIdentity();
}

//--------------------------------------------------
/*!
	@brief	描画
*/
void EditViewBase::paintEvent(QPaintEvent *event)
{
	// ここでのOpenGLの描画を可能にする
	makeCurrent();

	// クリア
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// OpenGLでの描画
	glScalef(dispScale_, dispScale_, dispScale_);
	drawRegion();
#if 0
	drawBoundingBox();
#endif

	setupViewport(width(), height());

	// QPainterによる描画（2D）
	QPainter painter(this);
	draw2D(&painter);
	painter.end();
}

//--------------------------------------------------
/*!
	@brief	領域描画
*/
void EditViewBase::drawRegion()
{
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.5);
	for(int i = 0; i < textureDatas_.size(); i++)
	{
		glPushMatrix();
		setRegionColor(i);
		glBindTexture(GL_TEXTURE_2D, textureDatas_.at(i).textureID);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-1.0f,  1.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f( 1.0f,  1.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f( 1.0f, -1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-1.0f, -1.0f, 0.0f);
		glEnd();
		glPopMatrix();
	}
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
}

void EditViewBase::setRegionColor(int index)
{
	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	int selectViewID = select->viewID;
	RegionLinkData* selectEnableLinkData = select->selectEnableLinkData;
	RegionLinkData* selectLinkData = select->selectLinkData;
	ClosedRegion* thisRegion = textureDatas_.at(index).regionPtr;
	ClosedRegion* selectRegion = select->selectRegion;
	ClosedRegion* selectEnableRegion = select->selectEnableRegion;

	// ドラッグ中はドラッグ元の選択可能領域を暗く表示しない
	// ドラッグ元の選択領域だけ暗くする
	bool isDrag = ObjectManager::getInstance()->isRegionLinking();

	// 自身が選択可能か
	bool isSelfSelectEnable = false;
	if(thisRegion == selectEnableRegion)
	{
		isSelfSelectEnable = true;
	}

	// 他方が選択可能か
	bool isLinkedRegionSelectEnable = false;
	if(selectViewID != -1 && selectEnableLinkData != NULL && !isDrag)
	{
		ClosedRegion* linkedRegion = selectEnableLinkData->getRegion(getViewID());
		if(thisRegion == linkedRegion)
		{
			isLinkedRegionSelectEnable = true;
		}
	}
	
	// 自身が選択状態か
	bool isSelfSelected = false;
	if(thisRegion == selectRegion)
	{
		isSelfSelected = true;
	}

	// 他方が選択状態か
	bool isLinkedRegionSelected = false;
	if(selectViewID != -1 && selectLinkData != NULL)
	{
		ClosedRegion* linkedRegion = selectLinkData->getRegion(getViewID());
		if(thisRegion == linkedRegion)
		{
			isLinkedRegionSelected = true;
		}
	}

	// 自身が選択可能、他方（対応付けられた領域）が選択可能、自身が選択状態、他方（対応付けられた領域）が選択状態のいずれか
	bool isSelected = false;
	if( isSelfSelectEnable  || 
		isLinkedRegionSelectEnable ||
		isSelfSelected ||
		isLinkedRegionSelected)
	{
		// 2014/06/25 暗くする必要はないのでコメントアウト
		//?isSelected = true;
	}
	if(isDispMatchColor_)
	{
		ClosedRegion* r = textureDatas_.at(index).regionPtr;
		RegionLinkDataManager* mgr = ObjectManager::getInstance()->getRegionLinkDataManager();
		QColor indexColor = mgr->getIndexColor(r);

		if(isSelected)
		{
			indexColor = indexColor.darker();
		}
		qglColor(indexColor);
	}
	else
	{
		if(isSelected)
		{
			QColor color = textureDatas_.at(index).regionColor.darker();
			qglColor(color);
		}
		else
		{
			qglColor(textureDatas_.at(index).regionColor);
		}
	}
}

//--------------------------------------------------
/*!
	@brief	境界ピクセル描画
*/
void EditViewBase::drawBoundaryLine()
{
	//
	// 選択領域に対応領域がある場合だけ描画する
	//

	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	RegionLinkData* selectLinkData = select->selectLinkData;
	if(!selectLinkData)
		return;

	ClosedRegion* selectRegion = select->selectRegion;
	ClosedRegion* thisSelectRegion = selectLinkData->getRegion(getViewID());

	if(!thisSelectRegion)
		return;

	// ２つ以上の対応領域があれば描画
	int linkNum = 0;
	for(int i = 0; i < VIEW_MAX; i++)
	{
		if(selectLinkData->getRegion(i))
		{
			linkNum++;
		}
	}
	if(linkNum < 2)
		return;

	float w = thisSelectRegion->getRegionMap().getWidth();
	float h = thisSelectRegion->getRegionMap().getHeight();
	QVector<Line*>* lines = thisSelectRegion->getBoundaryLines();
	glPushMatrix();
	glColor3f(1.0, 0.0, 0.0);
	
	for(int i = 0; i < lines->size(); i++)
	{
		Line* line = lines->at(i);
		glBegin(GL_LINES);
		for(int j = 0; j < line->getPoints().size() - 1; j++)
		{
			QVector2D p0 = line->getPoints().at(j);
			QVector2D p1 = line->getPoints().at(j+1);
			float x0 = 2.0f * p0.x() / w - 1.0f;
			float y0 = 2.0f * p0.y() / h - 1.0f;
			float x1 = 2.0f * p1.x() / w - 1.0f;
			float y1 = 2.0f * p1.y() / h - 1.0f;
			glVertex3f(x0, y0, 0.0f);
			glVertex3f(x1, y1, 0.0f);
		}
		glEnd();
	}
	
	glPopMatrix();

}

//--------------------------------------------------
/*!
	@brief	特徴点描画
*/
void EditViewBase::drawFeaturePoints(QPainter* painter)
{
	//
	// 選択領域に対応領域がある場合だけ描画する
	//

	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	RegionLinkData* selectLinkData = select->selectLinkData;
	if(!selectLinkData)
		return;

	ClosedRegion* selectRegion = select->selectRegion;
	ClosedRegion* thisSelectRegion = selectLinkData->getRegion(getViewID());

	if(!thisSelectRegion)
		return;

	// ２つ以上の対応領域があれば描画
	int linkNum = 0;
	for(int i = 0; i < VIEW_MAX; i++)
	{
		if(selectLinkData->getRegion(i))
		{
			linkNum++;
		}
	}
	if(linkNum < 2)
		return;

	float w = thisSelectRegion->getRegionMap().getWidth();
	float h = thisSelectRegion->getRegionMap().getHeight();
	const std::vector<IntVec::ivec2>& boundaries = thisSelectRegion->getBoundaryPixels();

	// 特徴点描画
	// スクリーン座標系で計算する
	const QVector<int>* featureIndices = thisSelectRegion->getFeaturePointIndices();
	bool* indices = new bool[boundaries.size()];
	for(int i = 0; i < boundaries.size(); i++)
	{
		indices[i] = false;
	}
	for(int i = 0; i < featureIndices->size(); i++)
	{
		int idx = featureIndices->at(i);
		indices[idx] = true;
	}

	// 他方の選択特徴点を求める
	int moveEnableFeaturePointIndex = (select->selectFeatureIndex != -1) ?
		featureIndices->at(select->selectFeatureIndex): -1;

	for(int i = 0; i < boundaries.size(); i++)
	{
		QBrush brush(Qt::SolidPattern);
		QPen pen(Qt::black);
		pen.setWidth(1);

		// 選択可能頂点
		if(i == selectPointIndex_)
		{
			brush.setColor(Qt::blue);
		}
		// 選択している特徴点
		else if(i == moveEnableFeaturePointIndex)
		{
			brush.setColor(Qt::red);
		}
		// 選択していない特徴点
		else if(indices[i])
		{
			brush.setColor(Qt::blue);
		}
		else
		{
			continue;
		}

		QVector2D p = QVector2D(boundaries.at(i).x, boundaries.at(i).y);
		float x = p.x() * dispScale_ - SELECT_POINT_SIZE / 2.0f;
		float y = (h - p.y()) * dispScale_ - SELECT_POINT_SIZE / 2.0f;

		painter->setPen(pen);
		painter->setBrush(brush);
		painter->drawEllipse(x, y, SELECT_POINT_SIZE, SELECT_POINT_SIZE);
	}

	delete [] indices;
}

void EditViewBase::drawRegionPolygon(QPainter& painter, QPolygon& polygon, QColor& color, QColor& edgeColor, 
	QPoint& pos, bool isEdgeDraw, QPointF scale)
{
	QPen pen;
	if(isEdgeDraw)
	{
		pen.setColor(edgeColor);
		pen.setWidth(2);
	}
	else
	{
		pen.setColor(color);
		pen.setWidth(1);
	}
	//QBrush brush(color, Qt::SolidPattern);
	QBrush brush(QColor(0,0,0,0), Qt::SolidPattern);
	painter.setPen(pen);
	painter.setBrush(brush);

	painter.save();
	painter.translate(pos);
	painter.scale(scale.x(), scale.y());
	painter.drawPolygon(polygon);
	painter.restore();
}
//--------------------------------------------------
/*!
	@brief	バウンディングボックス描画
*/
void EditViewBase::drawBoundingBox()
{
	for(int i = 0; i < textureDatas_.size(); i++)
	{
		QRectF rect = textureDatas_.at(i).boundingBox;
		glPushMatrix();
		glBegin(GL_LINE_LOOP);

		int regionID = textureDatas_.at(i).regionID;
		if(regionID == selectRegionID_)
		{
			glColor3f(1.0f, 0.0f, 0.0f);
		}
		else if(regionID == selectEnableRegionID_)
		{
			glColor3f(0.0f, 1.0f, 0.0f);
		}
		else
		{
			glColor3f(0.3f, 0.3f, 0.8f);
		}
		glVertex2f(rect.x(), rect.y());
		glVertex2f(rect.x() + rect.width(), rect.y());
		glVertex2f(rect.x() + rect.width(), rect.y() + rect.height());
		glVertex2f(rect.x(), rect.y() + rect.height());
		glEnd();
		glPopMatrix();
	}
}

//--------------------------------------------------
/*!
	@brief	QPainterによる描画（2D）
*/
void EditViewBase::draw2D(QPainter* painter)
{
	QString str = QString("MODE = %1").arg(ObjectManager::getInstance()->getEditMode());
	QString str2 = QString("Y = %1").arg(mousePos_.y());
	painter->setPen(Qt::green);
//	painter->drawText( 30, 30, str );
//	painter->drawText( 30, 50, str2 );
	

	// 選択ポリゴン描画
	drawRegionPolygon(*painter, selectEnablePolygon_, selectEnablePolygonColor_, sSelectEnableRegionEdgeColor, selectEnablePolygonPos_, true, QPointF(dispScale_, dispScale_));
	drawRegionPolygon(*painter, selectPolygon_, selectPolygonColor_, sSelectRegionEdgeColor, selectPolygonPos_, true, QPointF(dispScale_, dispScale_));
	
	// リンク用の線描画
	ObjectManager* mgr = ObjectManager::getInstance();
	if(mgr->isRegionLinking())
	{
		QPoint start = mgr->getDragStartGlobalPos() - pos();
		QPoint end = mgr->getDragEndGlobalPos() - pos();
		painter->setPen(QPen(sLinkingLineColor));
		painter->drawLine(start, end);
	}

	// 特徴点描画
	if(ObjectManager::getInstance()->getEditMode() == MODE_FEATUREPOINT_EDIT)
	{
		drawFeaturePoints(painter);
	}
}

//--------------------------------------------------
/*!
	@brief	マウス（押し）イベント
*/
void EditViewBase::mousePressEvent(QMouseEvent *event)
{
	/*
		event->button() == Qt::LeftButton
		などで押されたボタン取得
		マウス座標はevent->x(), event->y()などで取得
	*/

	
	switch(ObjectManager::getInstance()->getEditMode())
	{
	case MODE_REGION_MATCH:
		if(event->button() == Qt::LeftButton)
		{
			selectRegionID_ = selectEnableRegionID_;
			updateSelect();
			emit regionSelectEnabled();
			emit regionSelected();
		
			// ドラッグ検知用の位置保持
			startPos_ = event->pos();
			update();
		}
		break;

	case MODE_FEATUREPOINT_EDIT:
		if(event->button() == Qt::LeftButton)
		{
			selectRegionID_ = selectEnableRegionID_;
			updateSelect();
			emit regionSelectEnabled();
			emit regionSelected();
			update();
		}

		if(event->button() == Qt::RightButton)
		{
			if(moveEnableFeaturePointIndex_ == -1)
			{
				setFeaturePoint();
			}
			else
			{
				isMoveFeaturePoint_ = true;
			}
		}
		break;
	}
}

//--------------------------------------------------
/*!
	@brief	マウス（離し）イベント
*/
void EditViewBase::mouseReleaseEvent(QMouseEvent *event)
{
	switch(ObjectManager::getInstance()->getEditMode())
	{
	case MODE_REGION_MATCH:
		break;

	case MODE_FEATUREPOINT_EDIT:
		isMoveFeaturePoint_ = false;
		break;
	}
	
}

//--------------------------------------------------
/*!
	@brief	マウス移動イベント
*/
void EditViewBase::mouseMoveEvent(QMouseEvent *event)
{
	mousePos_ = event->pos();

	switch(ObjectManager::getInstance()->getEditMode())
	{
	case MODE_REGION_MATCH:
		{
			// 選択領域の更新
			hitTest();
			updateSelectEnable();
			emit regionSelectEnabled();

			// 領域対応付け用のドラッグ検知
			if(event->buttons() & Qt::LeftButton) 
			{ 
				int distance = (event->pos() - startPos_).manhattanLength();   
				if(distance >= QApplication::startDragDistance())    
				{
					startDrag(); 
				}     
			} 

		}
		break;

	case MODE_FEATUREPOINT_EDIT:
		{
			// 特徴点を選択するとき
			if(!isMoveFeaturePoint_)
			{
				// 選択領域の更新
				hitTest();
				updateSelectEnable();
				emit regionSelectEnabled();

				// 選択頂点の更新
				// 特徴点の移動と追加
				// 既存の特徴点の選択が優先される
				hitTest_FeaturePoints();
				if(moveEnableFeaturePointIndex_ == -1)
				{
					hitTest_Vertices();
				}
				else
				{
					selectPointIndex_ = -1;
				}
			}
			// 特徴点を移動させているとき
			else
			{
				movedFeaturePoint();
			}
		}
		break;
	}

	// 再描画リクエストを通知
	update();
}

//------------------------------------------------------
/*!
	@brief	キーボードイベント
*/
void EditViewBase::keyPressEvent(QKeyEvent *event)
{
	/*
		event->key() == Qt::Key_S
		などで押されたキーを取得
	*/

	//? test
	
	if(event->key() == Qt::Key_C && selectRegionID_ != -1)
	{
		createMatchedRegion();
	}
	
	// デプス修正テスト
	// 視線方向にデプスを足す
	if(event->key() == Qt::Key_Z && selectRegionID_ != -1)
	{
		ObjectManager* mgr = ObjectManager::getInstance();
		int dataIndex = getDataIndex(selectRegionID_);
		ClosedRegion* r = textureDatas_.at(dataIndex).regionPtr;
		QVector3D pos = r->getPos3D();

		QVector3D dir(0,0,-1);
		QVector2D rot;
		if(getViewID() == VIEW_MAIN)
		{
			rot = QVector2D(mgr->getSrcRotation());
		}
		else if(getViewID() == VIEW_SIDE_RIGHT)
		{
			rot = QVector2D(mgr->getDstRotation());
		}
		QMatrix4x4 matRotY;
		matRotY.rotate(-rot.y(), 0,1,0);
		QMatrix4x4 matRotX;
		matRotX.rotate(-rot.x(), 1,0,0);
		dir = matRotX * matRotY * dir;
		dir.normalize();
		pos += dir * 0.01f;
		r->setPos3D(pos);
		qDebug("x = %f, y = %f, z = %f", pos.x(), pos.y(), pos.z());
	}

	// 視線方向にデプスをひく
	if(event->key() == Qt::Key_A && selectRegionID_ != -1)
	{
		ObjectManager* mgr = ObjectManager::getInstance();
		int dataIndex = getDataIndex(selectRegionID_);
		ClosedRegion* r = textureDatas_.at(dataIndex).regionPtr;
		QVector3D pos = r->getPos3D();

		QVector3D dir(0,0,1);
		QVector2D rot;
		if(getViewID() == VIEW_MAIN)
		{
			rot = QVector2D(mgr->getSrcRotation());
		}
		else if(getViewID() == VIEW_SIDE_RIGHT)
		{
			rot = QVector2D(mgr->getDstRotation());
		}
		QMatrix4x4 matRotY;
		matRotY.rotate(-rot.y(), 0,1,0);
		QMatrix4x4 matRotX;
		matRotX.rotate(-rot.x(), 1,0,0);
		dir = matRotX * matRotY * dir;
		dir.normalize();
		pos += dir * 0.01f;
		r->setPos3D(pos);
		qDebug("x = %f, y = %f, z = %f", pos.x(), pos.y(), pos.z());
	}

}

//------------------------------------------------------
/*!
	@brief	ドラッグ開始
*/
void EditViewBase::startDrag()
{  
	if(selectRegionID_ == -1)
		return;

	QPoint globalPos = startPos_ + pos();
	ObjectManager::getInstance()->setDragStartGlobalPos(globalPos);
	ObjectManager::getInstance()->setDragEndGlobalPos(globalPos);
	ObjectManager::getInstance()->setRegionLinking(true);

	QMimeData *mimeData = new QMimeData;
	QByteArray ba; 
	ba.resize(1);
	ba[0] = 0;
	mimeData->setData("testData", ba);   
	QDrag *drag = new QDrag(this);  
	drag->setMimeData(mimeData);
	drag->exec(Qt::LinkAction);
}
//------------------------------------------------------
/*!
	@brief	ドラッグされたときのイベント
*/
void EditViewBase::dragEnterEvent(QDragEnterEvent *event)
{
	ObjectManager::getInstance()->setRegionLinking(true);
	event->setDropAction(Qt::LinkAction);
	event->accept();
}

//------------------------------------------------------
/*!
	@brief	ドラッグ中イベント
*/
void EditViewBase::dragMoveEvent(QDragMoveEvent *event)
{
	mousePos_ = event->pos();
	QPoint globalPos = mousePos_ + pos();
	ObjectManager::getInstance()->setDragEndGlobalPos(globalPos);

	event->accept();

	// 自分以外からのドラッグ＆ドロップなら
	if (event->source() != this)
	{
		event->setDropAction(Qt::LinkAction);

		if(ObjectManager::getInstance()->getEditMode() == MODE_REGION_MATCH)
		{
			hitTest();

			// updateSelectEnableはviewIDも変わってしまうので、ここではselectEnableRegion,selectEnableLinkDataを更新
			// ドラッグ中のビューIDはドラッグ元にしておかないと対応付けの時にうまくいかない
			SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
			int dataIndex = getDataIndex(selectEnableRegionID_);
			ClosedRegion* selectEnableRegion = (dataIndex != -1) ? textureDatas_.at(dataIndex).regionPtr : NULL;
			select->selectEnableLinkData = ( selectEnableRegion ) ? selectEnableRegion->getRegionLinkData() : NULL;
			select->selectEnableRegion = selectEnableRegion;
			
			if(selectEnableRegion)
			{
				selectEnablePolygonPos_ = makePolygon(*selectEnableRegion, selectEnablePolygonColor_, selectEnablePolygon_);
			}
		}
	}
	update();

	emit globalPosUpdated();
}


void EditViewBase::dragLeaveEvent(QDragLeaveEvent *event)
{
	ObjectManager::getInstance()->setRegionLinking(false);
	update();
	emit globalPosUpdated();
}

//------------------------------------------------------
/*!
	@brief	ドロップイベント
*/
void EditViewBase::dropEvent(QDropEvent *event)
{
	ObjectManager::getInstance()->setRegionLinking(false);
	mousePos_ = event->pos();
	event->accept();

	// 自分以外からのドラッグ＆ドロップなら 
	if (event->source() != this)
	{	
		event->setDropAction(Qt::LinkAction);
		
		// 独自データ抽出    
		QByteArray ba = event->mimeData()->data("testData");
		int testData = ba[0];
		
		hitTest();

		// 対応付け
		RegionLinkDataManager* linkMgr = ObjectManager::getInstance()->getRegionLinkDataManager();
		ClosedRegion* mainRegion = NULL;
		ClosedRegion* subRegion = NULL;
		int viewID;

		SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
		// サブからメインへ対応付け
		if(getViewID() == VIEW_MAIN)
		{
			viewID = select->viewID;
			subRegion = select->selectRegion;
			int dataIndex = getDataIndex(selectEnableRegionID_);
			mainRegion = (dataIndex != -1) ? textureDatas_.at(dataIndex).regionPtr : NULL;
			if(mainRegion)
			{
				linkMgr->link(mainRegion, subRegion, viewID);
			}
			// メインがないときは、サブに関連付けられたメイン領域との対応を外す
			else
			{
				RegionLinkData* linkData = subRegion->getRegionLinkData();
				if(linkData)
				{
					ClosedRegion* linkedMainRegion = linkData->getRegion(VIEW_MAIN);
					linkMgr->link(linkedMainRegion, NULL, viewID);
				}
			}
		}
		// メインからサブへ対応付け
		else
		{
			mainRegion = select->selectLinkData->getRegion(VIEW_MAIN);
			int dataIndex = getDataIndex(selectEnableRegionID_);
			subRegion = (dataIndex != -1) ? textureDatas_.at(dataIndex).regionPtr : NULL;
			viewID = getViewID();
			linkMgr->link(mainRegion, subRegion, viewID);
		}
		

		// 選択を更新
		selectRegionID_ = selectEnableRegionID_;
		updateSelect();
		updateSelectEnable();
		emit regionSelectEnabled();
		emit regionSelected();
	}
}


/*!
	@brief	選択情報の更新
*/

void EditViewBase::updateSelect()
{
	int viewID = getViewID();
	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	select->viewID = viewID;
	select->selectedViewID = viewID;

	int dataIndex = getDataIndex(selectRegionID_);
	ClosedRegion* selectRegion = (dataIndex != -1) ? textureDatas_.at(dataIndex).regionPtr : NULL;
	select->selectLinkData = ( selectRegion ) ? selectRegion->getRegionLinkData() : NULL;
	select->selectRegion = selectRegion;

	// ポリゴン作成
	if(selectRegion)
	{
		selectPolygonPos_ = makePolygon(*selectRegion, selectPolygonColor_, selectPolygon_);
	}
	else
	{
		selectPolygon_.clear();
	}
}

void EditViewBase::updateSelectEnable()
{
	int viewID = getViewID();
	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	select->viewID = viewID;

	int dataIndex = getDataIndex(selectEnableRegionID_);
	ClosedRegion* selectEnableRegion = (dataIndex != -1) ? textureDatas_.at(dataIndex).regionPtr : NULL;
	select->selectEnableLinkData = ( selectEnableRegion ) ? selectEnableRegion->getRegionLinkData() : NULL;
	select->selectEnableRegion = selectEnableRegion;

	// ポリゴン作成
	if(selectEnableRegion)
	{
		selectEnablePolygonPos_ = makePolygon(*selectEnableRegion, selectEnablePolygonColor_, selectEnablePolygon_);
	}
	else
	{
		selectEnablePolygon_.clear();
	}
}


/*!
	@brief	ヒットテスト
*/
void EditViewBase::hitTest()
{
	AnimeFrame* frame = getAnimeFrame();
	if(!frame)
		return;

	QVector<int> hitIndices;
	float x = ((float)mousePos_.x() / (float)width()) - 0.5f;
	float y = 0.5f - ((float)mousePos_.y() / (float)height());

	int px = (x/dispScale_ + 0.5) * width();
	int py = (y/dispScale_ + 0.5) * height();

	
	std::vector<ClosedRegion*> regions = frame->getRegions();
	for(int i = 0; i < regions.size(); i++)
	{
		ClosedRegion* r = regions.at(i);
		
		RegionMap& regionMap = r->getRegionMap();
		if( 0 <= px && px < regionMap.getWidth()  && 0 <= py && py < regionMap.getHeight())
		{
			IntVec::ubvec4 color = regionMap(px, py);
			if(color.r != 0 || color.g != 0 || color.b != 0)
			{
				hitIndices.append(i);
			}
		}
	}

	float minDist = 0xffff;
	int minIndex = -1;
	for(int i = 0; i < hitIndices.size(); i++)
	{
		int index = hitIndices.at(i);
		QPointF center = textureDatas_.at(index).boundingBox.center();
		QVector2D vc(center);
		vc *= dispScale_;
		QVector2D vp(x, y);
		float dist = QVector2D(vc - vp).length();
		if(dist < minDist)
		{
			minDist = dist;
			minIndex = index;
		}
	}
	selectEnableRegionID_ = (minIndex != -1) ? textureDatas_.at(minIndex).regionID : -1;
}

/*!
	@brief	特徴点とのヒットテスト
*/
void EditViewBase::hitTest_FeaturePoints()
{
	moveEnableFeaturePointIndex_ = -1;
	//
	// 選択領域に対応領域がある場合だけヒットテストを行う
	//

	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	select->selectFeatureIndex = -1;
	RegionLinkData* selectLinkData = select->selectLinkData;
	if(!selectLinkData)
		return;

	ClosedRegion* selectRegion = select->selectRegion;
	ClosedRegion* thisSelectRegion = selectLinkData->getRegion(getViewID());

	if(!thisSelectRegion)
		return;

	// ２つ以上の対応領域があればヒットテストを行う
	int linkNum = 0;
	for(int i = 0; i < VIEW_MAX; i++)
	{
		if(selectLinkData->getRegion(i))
		{
			linkNum++;
		}
	}
	if(linkNum < 2)
		return;

	float w = thisSelectRegion->getRegionMap().getWidth();
	float h = thisSelectRegion->getRegionMap().getHeight();
	const std::vector<IntVec::ivec2>& boundaries = thisSelectRegion->getBoundaryPixels();

	// スクリーン座標系で計算する
	float minLength = 0xffff;
	QVector2D mouse(mousePos_.x(), height() - mousePos_.y());
	QVector2D windowCenter(w / 2.0f, h / 2.0f);
	QVector2D dispCenter = windowCenter * dispScale_;

	const QVector<int>* featurePointIndices = thisSelectRegion->getFeaturePointIndices();
	for(int i = 0; i < featurePointIndices->size(); i++)
	{
		int pixelIndex = featurePointIndices->at(i);
		QVector2D v = QVector2D(boundaries.at(pixelIndex).x, boundaries.at(pixelIndex).y);
		v *= dispScale_;
		v += windowCenter - dispCenter;
		float length = (mouse - v).length();
		if(length < RADIUS_SIZE && length < minLength)
		{
			minLength = length;
			moveEnableFeaturePointIndex_ = pixelIndex;
			select->selectFeatureIndex = i;
		}
	}
}

/*!
	@brief 特徴点の移動
*/
void EditViewBase::movedFeaturePoint()
{
	int movedFeaturePointIndex = -1;
	//
	// 選択領域に対応領域がある場合だけヒットテストを行う
	//

	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	RegionLinkData* selectLinkData = select->selectLinkData;
	if(!selectLinkData)
		return;

	ClosedRegion* selectRegion = select->selectRegion;
	ClosedRegion* thisSelectRegion = selectLinkData->getRegion(getViewID());

	if(!thisSelectRegion)
		return;

	// ２つ以上の対応領域があればヒットテストを行う
	int linkNum = 0;
	for(int i = 0; i < VIEW_MAX; i++)
	{
		if(selectLinkData->getRegion(i))
		{
			linkNum++;
		}
	}
	if(linkNum < 2)
		return;

	float w = thisSelectRegion->getRegionMap().getWidth();
	float h = thisSelectRegion->getRegionMap().getHeight();
	const std::vector<IntVec::ivec2>& boundaries = thisSelectRegion->getBoundaryPixels();

	// スクリーン座標系で計算する
	float minLength = 0xffff;
	QVector2D mouse(mousePos_.x(), height() - mousePos_.y());
	QVector2D windowCenter(w / 2.0f, h / 2.0f);
	QVector2D dispCenter = windowCenter * dispScale_;

	// 移動可能な区間は、前後の特徴点に挟まれた区間
	// この区間の頂点だけと判定
	const QVector<int>* featureIndices = thisSelectRegion->getFeaturePointIndices();
	int selectFeaturePointIndex = featureIndices->at(select->selectFeatureIndex);

	// 通常の判定
	if(selectFeaturePointIndex != 0)
	{
		Q_ASSERT(select->selectFeatureIndex > 0);
		int prevPointIndex = featureIndices->at(select->selectFeatureIndex - 1);
		int nextPointIndex = (select->selectFeatureIndex == featureIndices->size() - 1) ? 
			boundaries.size() - 1 : featureIndices->at(select->selectFeatureIndex + 1);

		for(int i = prevPointIndex + 1; i < nextPointIndex; i++)
		{
			QVector2D v = QVector2D(boundaries.at(i).x, boundaries.at(i).y);
			v *= dispScale_;
			v += windowCenter - dispCenter;
			float length = (mouse - v).length();
			if(length < RADIUS_SIZE && length < minLength)
			{
				minLength = length;
				movedFeaturePointIndex = i;
			}
		}

		// 特徴点データ書き換え
		if(movedFeaturePointIndex != -1)
		{
			thisSelectRegion->setFeaturePoint(select->selectFeatureIndex, movedFeaturePointIndex);
			selectLinkData->createLines();
		}
	}
	// 0番目の特徴点を動かす時は境界ピクセルの順番も動かす必要がある 
	else
	{
		// 動ける範囲は0-1と最後の特徴点-最後の頂点の２つの区間
		int nextPointIndex = (featureIndices->size() <= 1) ? boundaries.size() - 1 : featureIndices->at(1);
		int endPoitIndex = (featureIndices->size() <= 1) ? boundaries.size() - 1 : featureIndices->last();
		for(int i = 0; i < nextPointIndex; i++)
		{
			QVector2D v = QVector2D(boundaries.at(i).x, boundaries.at(i).y);
			v *= dispScale_;
			v += windowCenter - dispCenter;
			float length = (mouse - v).length();
			if(length < RADIUS_SIZE && length < minLength)
			{
				minLength = length;
				movedFeaturePointIndex = i;
			}
		}

		for(int i = endPoitIndex+1; i < boundaries.size()-1; i++)
		{
			QVector2D v = QVector2D(boundaries.at(i).x, boundaries.at(i).y);
			v *= dispScale_;
			v += windowCenter - dispCenter;
			float length = (mouse - v).length();
			if(length < RADIUS_SIZE && length < minLength)
			{
				minLength = length;
				movedFeaturePointIndex = i;
			}
		}

		// 特徴点データ書き換え
		if(movedFeaturePointIndex != -1)
		{
			int bufSize = featureIndices->size();
			int* buf = new int[ bufSize ];
			for(int i = 0; i < featureIndices->size(); i++)
			{
				buf[i] = featureIndices->at(i);
			}

			// 始点の位置を変える
			thisSelectRegion->setBoundaryStartPoint(movedFeaturePointIndex);
			
			// 特徴点データでずれた分を修正
			thisSelectRegion->resetFeaturePoint();
			for(int i = 1; i < bufSize; i++)
			{
				int newPointIndex = (buf[i] - movedFeaturePointIndex + boundaries.size()) % boundaries.size();
				thisSelectRegion->addFeaturePoint(newPointIndex);
			}
			
			selectLinkData->createLines();

			delete [] buf;
		}
#if 0
		// 動ける範囲は0-1と最後の特徴点-最後の頂点の２つの区間
		int nextPointIndex = (features->size() <= 1) ? boundaries.size() - 1 : features->at(1) * boundaries.size();
		int endPoitIndex = (features->size() <= 1) ? boundaries.size() - 1 : features->last() * boundaries.size();
		for(int i = 0; i < nextPointIndex; i++)
		{
			QVector2D v = QVector2D(boundaries.at(i).x, boundaries.at(i).y);
			v *= dispScale_;
			v += windowCenter - dispCenter;
			float length = (mouse - v).length();
			if(length < RADIUS_SIZE && length < minLength)
			{
				minLength = length;
				movedFeaturePointIndex_ = i;
			}
		}

		for(int i = endPoitIndex; i < boundaries.size()-1; i++)
		{
			QVector2D v = QVector2D(boundaries.at(i).x, boundaries.at(i).y);
			v *= dispScale_;
			v += windowCenter - dispCenter;
			float length = (mouse - v).length();
			if(length < RADIUS_SIZE && length < minLength)
			{
				minLength = length;
				movedFeaturePointIndex_ = i;
			}
		}

		// 特徴点データ書き換え
		if(movedFeaturePointIndex_ != -1)
		{
			int* buf = new int[ features->size() ];
			for(int i = 0; i < features->size(); i++)
			{
				int featurePointIndex = qFloor(features->at(i) * boundaries.size() + 0.5);
				buf[i] = featurePointIndex;
			}

			// 始点の位置を変える
			thisSelectRegion->setBoundaryStartPoint(movedFeaturePointIndex_);
			
			// 特徴点データでずれた分を修正
			for(int i = 1; i < features->size(); i++)
			{
				int newPointIndex = (buf[i] - movedFeaturePointIndex_ + boundaries.size()) % boundaries.size();
				float newRatio = (float)newPointIndex / (float)boundaries.size();
				
				qDebug("old idx %d, new idx %d, move %d, oldRatio %f, new ratio %f", buf[i], newPointIndex, movedFeaturePointIndex_, features->at(i), newRatio);
				thisSelectRegion->setFeaturePoint(i, newRatio);
			}

			selectLinkData->createLines();

			delete [] buf;
		}
#endif
	}

	

}

/*!
	@brief	選択領域の境界上の点とのヒットテスト
*/
void EditViewBase::hitTest_Vertices()
{
	selectPointIndex_ = -1;

	//
	// 選択領域に対応領域がある場合だけヒットテストを行う
	//

	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	RegionLinkData* selectLinkData = select->selectLinkData;
	if(!selectLinkData)
		return;

	ClosedRegion* selectRegion = select->selectRegion;
	ClosedRegion* thisSelectRegion = selectLinkData->getRegion(getViewID());

	if(!thisSelectRegion)
		return;

	// ２つ以上の対応領域があればヒットテストを行う
	int linkNum = 0;
	for(int i = 0; i < VIEW_MAX; i++)
	{
		if(selectLinkData->getRegion(i))
		{
			linkNum++;
		}
	}
	if(linkNum < 2)
		return;

	float w = thisSelectRegion->getRegionMap().getWidth();
	float h = thisSelectRegion->getRegionMap().getHeight();
	const std::vector<IntVec::ivec2>& boundaries = thisSelectRegion->getBoundaryPixels();

	// スクリーン座標系で計算する
	float minLength = 0xffff;
	QVector2D mouse(mousePos_.x(), height() - mousePos_.y());
	QVector2D windowCenter(w / 2.0f, h / 2.0f);
	QVector2D dispCenter = windowCenter * dispScale_;

	for(int i = 0; i < boundaries.size(); i++)
	{
		QVector2D v = QVector2D(boundaries.at(i).x, boundaries.at(i).y);
		v *= dispScale_;
		v += windowCenter - dispCenter;
		float length = (mouse - v).length();
		if(length < RADIUS_SIZE && length < minLength)
		{
			minLength = length;
			selectPointIndex_ = i;
		}
	}
}

void EditViewBase::makeTextures()
{
	makeCurrent();
	deleteTextures();

	AnimeFrame* frame = getAnimeFrame();
	std::vector<ClosedRegion*> regions = frame->getRegions();

	for(int i = 0; i < regions.size(); i++)
	{
		// テクスチャ取得
		RegionTextureData data;
		ClosedRegion* r = regions.at(i);
		int w, h;
		w = r->getRegionMap().getWidth();
		h = r->getRegionMap().getHeight();

		// 描画用に白黒のマップ作成
		RegionMap& regionMap = r->getRegionMap();
		RegionMap* mapMono = new RegionMap;
		mapMono->allocate(w, h);
		for(int y = 0; y < h; y++)
		{
			for(int x = 0; x < w; x++)
			{
				IntVec::ubvec4 white(255,255,255,255);
				IntVec::ubvec4 black(0,0,0,0);
				IntVec::ubvec4 color = regionMap(x, y);
				if(color.r != 0 || color.g != 0 || color.b !=0)
				{
					mapMono->setValue(x, y, white);
				}
				else
				{
					mapMono->setValue(x, y, black);
				}
			}
		}

		GLuint id;
		glGenTextures(1, &id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);	
		glBindTexture(GL_TEXTURE_2D, id );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, mapMono->getData());	
		glBindTexture(GL_TEXTURE_2D, 0);

		// バウンディングボックス計算
		// ここで正規化しておく
		IntVec::ivec2 bBoxMax = r->getBboxMax();
		IntVec::ivec2 bBoxMin = r->getBboxMin();
		float bw = bBoxMax.x - bBoxMin.x;
		float bh = bBoxMax.y - bBoxMin.y;
		QRectF rect((bBoxMin.x / (float)w) - 0.5f , (bBoxMin.y / (float)h) - 0.5f, bw / (float)w, bh / (float)h);

		// 各種データセット
		IntVec::ubvec3 regionColor = r->getRegionColor();
		data.regionColor = QColor(regionColor.r, regionColor.g, regionColor.b);
		data.boundingBox = rect;
		data.textureID = id;
		data.regionID = r->getID();
		data.regionMapMono = mapMono;
		data.regionPtr = r;
		textureDatas_.append(data);
	}
}

void EditViewBase::deleteTextures()
{
	makeCurrent();

	for(int i = 0; i < textureDatas_.size(); i++)
	{
		GLuint id = textureDatas_.at(i).textureID;
		glDeleteTextures(1, &id);
		RegionMap* map = textureDatas_.at(i).regionMapMono;
		if(map)
		{
			delete map;
		}
	}
	textureDatas_.clear();
}

/*!
	@brief	ポリゴン作成
*/
QPoint EditViewBase::makePolygon(ClosedRegion& r, QColor& outColor, QPolygon& outPolygon)
{
	// 領域のデータは左下が(0,0)、右上が(w,h)となっているので、
	// バウンディングボックスの中心が0となるように変換する
	// またQtの座標系はY座標が逆になるので変換する

	int w = r.getRegionMap().getWidth();
	int h = r.getRegionMap().getHeight();

	QVector2D center;
	center.setX( (r.getBboxMax().x + r.getBboxMin().x) / 2.0f);
	center.setY( (r.getBboxMax().y + r.getBboxMin().y) / 2.0f);
	QVector2D qtCenter(center.x(), h - center.y() - 1);

	if(isDispMatchColor_)
	{
		RegionLinkDataManager* mgr = ObjectManager::getInstance()->getRegionLinkDataManager();
		QColor indexColor = mgr->getIndexColor(&r);
		outColor.setRed(indexColor.red());
		outColor.setGreen(indexColor.green());
		outColor.setBlue(indexColor.blue());
		outColor.setAlpha(255);
	}
	else
	{
		outColor.setRed(r.getRegionColor().r);
		outColor.setGreen(r.getRegionColor().g);
		outColor.setBlue(r.getRegionColor().b);
		outColor.setAlpha(255);
	}

	outPolygon.clear();
	std::vector<IntVec::ivec2>& boundaryPixels = r.getBoundaryPixels();
	for(int i = 0; i < boundaryPixels.size(); i++)
	{
		float x = boundaryPixels.at(i).x - qtCenter.x();
		float y = h - boundaryPixels.at(i).y - 1 - qtCenter.y();
		outPolygon.append(QPoint(x, y));
	}

	// 選択領域ポリゴンの初期位置セット
	float x = (r.getBboxMax().x + r.getBboxMin().x) / 2.0f;
	float y = h - ((r.getBboxMax().y + r.getBboxMin().y) / 2.0f) - 1;
	
	return QPoint(x, y);
}

/*!
	@brief	ポリゴンクリア
*/
void EditViewBase::clearPolygon()
{
	selectPolygon_.clear();
	selectEnablePolygon_.clear();
}

/*!
	@brief	特徴点セット
*/
void EditViewBase::setFeaturePoint()
{
	if(selectPointIndex_ == -1)
		return;

	// 選択頂点の割合を計算
	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	RegionLinkData* selectLinkData = select->selectLinkData;
	if(!selectLinkData)
		return;

	ClosedRegion* selectRegion = select->selectRegion;
	ClosedRegion* thisSelectRegion = selectLinkData->getRegion(getViewID());

	if(!thisSelectRegion)
		return;

	// ２つ以上の対応領域があれば特徴点をセットする
	int linkNum = 0;
	for(int i = 0; i < VIEW_MAX; i++)
	{
		if(selectLinkData->getRegion(i))
		{
			linkNum++;
		}
	}
	if(linkNum < 2)
		return;

	float w = thisSelectRegion->getRegionMap().getWidth();
	float h = thisSelectRegion->getRegionMap().getHeight();
	const std::vector<IntVec::ivec2>& boundaries = thisSelectRegion->getBoundaryPixels();

	// 選択領域に特徴点セット
	int dataIndex = thisSelectRegion->addFeaturePoint(selectPointIndex_);

	// 対応領域に特徴点セット
	if(dataIndex == -1)
		return;

	float ratio = (float)selectPointIndex_ / (float)boundaries.size();
	for(int i = 0; i < VIEW_MAX; i++)
	{
		if(i == getViewID())
			continue;
		ClosedRegion* linkedRegion = selectLinkData->getRegion(i);
		if(linkedRegion)
		{
			linkedRegion->addFeaturePoint(dataIndex, ratio);
		}
	}

#if 0 //?
	float ratio = (float)selectPointIndex_ / (float)boundaries.size();
	
	// 選択領域に特徴点セット
	int dataIndex = thisSelectRegion->addFeaturePoint(ratio);

	// 対応領域に特徴点セット
	// 割合は前後の割合の中間値をセットする
	if(dataIndex == -1)
		return;

	for(int i = 0; i < VIEW_MAX; i++)
	{
		if(i == getViewID())
			continue;
		ClosedRegion* linkedRegion = selectLinkData->getRegion(i);
		if(linkedRegion)
		{
			linkedRegion->addFeaturePoint(dataIndex, ratio);
		}
	}
#endif
	// ライン作成
	selectLinkData->createLines();
}

void EditViewBase::closeEvent(QCloseEvent *event)
{
	emit closed();
}

void EditViewBase::rebuildTextures()
{
	makeTextures();
	update();
}

/*!
	@brief	対応領域がない場合、他方に領域を作成
*/
void EditViewBase::createMatchedRegion()
{
	int dataIndex = getDataIndex(selectRegionID_);
	ClosedRegion* r = textureDatas_.at(dataIndex).regionPtr;
	if(!r)
		return;

	ObjectManager::getInstance()->createMatchedRegion(r, getViewID());

	emit matchedRegionCreated();
}

int EditViewBase::getDataIndex(int regionID)
{
	for(int i = 0; i < textureDatas_.size(); i++)
	{
		if(textureDatas_.at(i).regionID == regionID)
		{
			return i;
		}
	}

	return -1;
}

void EditViewBase::rebuildPolygons()
{
	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	int selectViewID = select->viewID;
	RegionLinkData* selectEnableLinkData = select->selectEnableLinkData;
	RegionLinkData* selectLinkData = select->selectLinkData;
	ClosedRegion* selectRegion = select->selectRegion;
	ClosedRegion* selectEnableRegion = select->selectEnableRegion;

	bool isDrag = ObjectManager::getInstance()->isRegionLinking();

	// 他方で選択可能な領域に対応付けられた領域
	ClosedRegion* thisSelectEnableRegion = NULL;
	if(selectViewID != -1 && selectEnableLinkData != NULL)
	{
		thisSelectEnableRegion = selectEnableLinkData->getRegion(getViewID());
	}
	
	// 他方で選択した領域に対応付けられた領域
	ClosedRegion* thisSelectRegion = NULL;
	if(selectViewID != -1 && selectLinkData != NULL)
	{
		thisSelectRegion = selectLinkData->getRegion(getViewID());
	}

	// ポリゴン作成
	selectPolygon_.clear();
	if(thisSelectRegion)
	{
		selectPolygonPos_ = makePolygon(*thisSelectRegion, selectPolygonColor_, selectPolygon_);
	}

	selectEnablePolygon_.clear();
	if(thisSelectEnableRegion)
	{
		selectEnablePolygonPos_ = makePolygon(*thisSelectEnableRegion, selectEnablePolygonColor_, selectEnablePolygon_);
	}

	update();
}
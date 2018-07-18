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
	@brief	�R���X�g���N�^
*/
EditViewBase::EditViewBase(QWidget *parent)
{
	// OpenGL���g���Ƃ���false�ɂ���
	setAutoFillBackground(false);

	// �}�E�X�{�^���������ĂȂ��Ă�MouseMove�C�x���g������悤�ɂȂ�
	setMouseTracking(true);

	// �L�[�{�[�h�Ƀt�H�[�J�X����
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

	// �h���b�O���h���b�v��L���ɂ���
	setAcceptDrops(true);
}

//--------------------------------------------------
/*!
	@brief	�f�X�g���N�^
*/
EditViewBase::~EditViewBase()
{
	deleteTextures();
}

//--------------------------------------------------
/*!
	@brief	������
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
	@brief	�t�@�C���ǂ݌�̏�����
*/
void EditViewBase::initImage()
{
	// �f�t�H���g�E�B���h�E�T�C�Y�ݒ�
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
	@brief	�r���[�|�[�g�ݒ�
*/
void EditViewBase::setupViewport(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, +1.0, -1.0, 1.0, 1.0, -1.0); // �v���W�F�N�V�����s���P�ʍs��ɂ��邽�߁A���̒l���Z�b�g
	glMatrixMode(GL_MODELVIEW);
}
//--------------------------------------------------
/*!
	@brief	���T�C�Y
*/
void EditViewBase::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);
	glLoadIdentity();
}

//--------------------------------------------------
/*!
	@brief	�`��
*/
void EditViewBase::paintEvent(QPaintEvent *event)
{
	// �����ł�OpenGL�̕`����\�ɂ���
	makeCurrent();

	// �N���A
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// OpenGL�ł̕`��
	glScalef(dispScale_, dispScale_, dispScale_);
	drawRegion();
#if 0
	drawBoundingBox();
#endif

	setupViewport(width(), height());

	// QPainter�ɂ��`��i2D�j
	QPainter painter(this);
	draw2D(&painter);
	painter.end();
}

//--------------------------------------------------
/*!
	@brief	�̈�`��
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

	// �h���b�O���̓h���b�O���̑I���\�̈���Â��\�����Ȃ�
	// �h���b�O���̑I��̈悾���Â�����
	bool isDrag = ObjectManager::getInstance()->isRegionLinking();

	// ���g���I���\��
	bool isSelfSelectEnable = false;
	if(thisRegion == selectEnableRegion)
	{
		isSelfSelectEnable = true;
	}

	// �������I���\��
	bool isLinkedRegionSelectEnable = false;
	if(selectViewID != -1 && selectEnableLinkData != NULL && !isDrag)
	{
		ClosedRegion* linkedRegion = selectEnableLinkData->getRegion(getViewID());
		if(thisRegion == linkedRegion)
		{
			isLinkedRegionSelectEnable = true;
		}
	}
	
	// ���g���I����Ԃ�
	bool isSelfSelected = false;
	if(thisRegion == selectRegion)
	{
		isSelfSelected = true;
	}

	// �������I����Ԃ�
	bool isLinkedRegionSelected = false;
	if(selectViewID != -1 && selectLinkData != NULL)
	{
		ClosedRegion* linkedRegion = selectLinkData->getRegion(getViewID());
		if(thisRegion == linkedRegion)
		{
			isLinkedRegionSelected = true;
		}
	}

	// ���g���I���\�A�����i�Ή��t����ꂽ�̈�j���I���\�A���g���I����ԁA�����i�Ή��t����ꂽ�̈�j���I����Ԃ̂����ꂩ
	bool isSelected = false;
	if( isSelfSelectEnable  || 
		isLinkedRegionSelectEnable ||
		isSelfSelected ||
		isLinkedRegionSelected)
	{
		// 2014/06/25 �Â�����K�v�͂Ȃ��̂ŃR�����g�A�E�g
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
	@brief	���E�s�N�Z���`��
*/
void EditViewBase::drawBoundaryLine()
{
	//
	// �I��̈�ɑΉ��̈悪����ꍇ�����`�悷��
	//

	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	RegionLinkData* selectLinkData = select->selectLinkData;
	if(!selectLinkData)
		return;

	ClosedRegion* selectRegion = select->selectRegion;
	ClosedRegion* thisSelectRegion = selectLinkData->getRegion(getViewID());

	if(!thisSelectRegion)
		return;

	// �Q�ȏ�̑Ή��̈悪����Ε`��
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
	@brief	�����_�`��
*/
void EditViewBase::drawFeaturePoints(QPainter* painter)
{
	//
	// �I��̈�ɑΉ��̈悪����ꍇ�����`�悷��
	//

	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	RegionLinkData* selectLinkData = select->selectLinkData;
	if(!selectLinkData)
		return;

	ClosedRegion* selectRegion = select->selectRegion;
	ClosedRegion* thisSelectRegion = selectLinkData->getRegion(getViewID());

	if(!thisSelectRegion)
		return;

	// �Q�ȏ�̑Ή��̈悪����Ε`��
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

	// �����_�`��
	// �X�N���[�����W�n�Ōv�Z����
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

	// �����̑I������_�����߂�
	int moveEnableFeaturePointIndex = (select->selectFeatureIndex != -1) ?
		featureIndices->at(select->selectFeatureIndex): -1;

	for(int i = 0; i < boundaries.size(); i++)
	{
		QBrush brush(Qt::SolidPattern);
		QPen pen(Qt::black);
		pen.setWidth(1);

		// �I���\���_
		if(i == selectPointIndex_)
		{
			brush.setColor(Qt::blue);
		}
		// �I�����Ă�������_
		else if(i == moveEnableFeaturePointIndex)
		{
			brush.setColor(Qt::red);
		}
		// �I�����Ă��Ȃ������_
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
	@brief	�o�E���f�B���O�{�b�N�X�`��
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
	@brief	QPainter�ɂ��`��i2D�j
*/
void EditViewBase::draw2D(QPainter* painter)
{
	QString str = QString("MODE = %1").arg(ObjectManager::getInstance()->getEditMode());
	QString str2 = QString("Y = %1").arg(mousePos_.y());
	painter->setPen(Qt::green);
//	painter->drawText( 30, 30, str );
//	painter->drawText( 30, 50, str2 );
	

	// �I���|���S���`��
	drawRegionPolygon(*painter, selectEnablePolygon_, selectEnablePolygonColor_, sSelectEnableRegionEdgeColor, selectEnablePolygonPos_, true, QPointF(dispScale_, dispScale_));
	drawRegionPolygon(*painter, selectPolygon_, selectPolygonColor_, sSelectRegionEdgeColor, selectPolygonPos_, true, QPointF(dispScale_, dispScale_));
	
	// �����N�p�̐��`��
	ObjectManager* mgr = ObjectManager::getInstance();
	if(mgr->isRegionLinking())
	{
		QPoint start = mgr->getDragStartGlobalPos() - pos();
		QPoint end = mgr->getDragEndGlobalPos() - pos();
		painter->setPen(QPen(sLinkingLineColor));
		painter->drawLine(start, end);
	}

	// �����_�`��
	if(ObjectManager::getInstance()->getEditMode() == MODE_FEATUREPOINT_EDIT)
	{
		drawFeaturePoints(painter);
	}
}

//--------------------------------------------------
/*!
	@brief	�}�E�X�i�����j�C�x���g
*/
void EditViewBase::mousePressEvent(QMouseEvent *event)
{
	/*
		event->button() == Qt::LeftButton
		�Ȃǂŉ����ꂽ�{�^���擾
		�}�E�X���W��event->x(), event->y()�ȂǂŎ擾
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
		
			// �h���b�O���m�p�̈ʒu�ێ�
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
	@brief	�}�E�X�i�����j�C�x���g
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
	@brief	�}�E�X�ړ��C�x���g
*/
void EditViewBase::mouseMoveEvent(QMouseEvent *event)
{
	mousePos_ = event->pos();

	switch(ObjectManager::getInstance()->getEditMode())
	{
	case MODE_REGION_MATCH:
		{
			// �I��̈�̍X�V
			hitTest();
			updateSelectEnable();
			emit regionSelectEnabled();

			// �̈�Ή��t���p�̃h���b�O���m
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
			// �����_��I������Ƃ�
			if(!isMoveFeaturePoint_)
			{
				// �I��̈�̍X�V
				hitTest();
				updateSelectEnable();
				emit regionSelectEnabled();

				// �I�𒸓_�̍X�V
				// �����_�̈ړ��ƒǉ�
				// �����̓����_�̑I�����D�悳���
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
			// �����_���ړ������Ă���Ƃ�
			else
			{
				movedFeaturePoint();
			}
		}
		break;
	}

	// �ĕ`�惊�N�G�X�g��ʒm
	update();
}

//------------------------------------------------------
/*!
	@brief	�L�[�{�[�h�C�x���g
*/
void EditViewBase::keyPressEvent(QKeyEvent *event)
{
	/*
		event->key() == Qt::Key_S
		�Ȃǂŉ����ꂽ�L�[���擾
	*/

	//? test
	
	if(event->key() == Qt::Key_C && selectRegionID_ != -1)
	{
		createMatchedRegion();
	}
	
	// �f�v�X�C���e�X�g
	// ���������Ƀf�v�X�𑫂�
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

	// ���������Ƀf�v�X���Ђ�
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
	@brief	�h���b�O�J�n
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
	@brief	�h���b�O���ꂽ�Ƃ��̃C�x���g
*/
void EditViewBase::dragEnterEvent(QDragEnterEvent *event)
{
	ObjectManager::getInstance()->setRegionLinking(true);
	event->setDropAction(Qt::LinkAction);
	event->accept();
}

//------------------------------------------------------
/*!
	@brief	�h���b�O���C�x���g
*/
void EditViewBase::dragMoveEvent(QDragMoveEvent *event)
{
	mousePos_ = event->pos();
	QPoint globalPos = mousePos_ + pos();
	ObjectManager::getInstance()->setDragEndGlobalPos(globalPos);

	event->accept();

	// �����ȊO����̃h���b�O���h���b�v�Ȃ�
	if (event->source() != this)
	{
		event->setDropAction(Qt::LinkAction);

		if(ObjectManager::getInstance()->getEditMode() == MODE_REGION_MATCH)
		{
			hitTest();

			// updateSelectEnable��viewID���ς���Ă��܂��̂ŁA�����ł�selectEnableRegion,selectEnableLinkData���X�V
			// �h���b�O���̃r���[ID�̓h���b�O���ɂ��Ă����Ȃ��ƑΉ��t���̎��ɂ��܂������Ȃ�
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
	@brief	�h���b�v�C�x���g
*/
void EditViewBase::dropEvent(QDropEvent *event)
{
	ObjectManager::getInstance()->setRegionLinking(false);
	mousePos_ = event->pos();
	event->accept();

	// �����ȊO����̃h���b�O���h���b�v�Ȃ� 
	if (event->source() != this)
	{	
		event->setDropAction(Qt::LinkAction);
		
		// �Ǝ��f�[�^���o    
		QByteArray ba = event->mimeData()->data("testData");
		int testData = ba[0];
		
		hitTest();

		// �Ή��t��
		RegionLinkDataManager* linkMgr = ObjectManager::getInstance()->getRegionLinkDataManager();
		ClosedRegion* mainRegion = NULL;
		ClosedRegion* subRegion = NULL;
		int viewID;

		SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
		// �T�u���烁�C���֑Ή��t��
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
			// ���C�����Ȃ��Ƃ��́A�T�u�Ɋ֘A�t����ꂽ���C���̈�Ƃ̑Ή����O��
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
		// ���C������T�u�֑Ή��t��
		else
		{
			mainRegion = select->selectLinkData->getRegion(VIEW_MAIN);
			int dataIndex = getDataIndex(selectEnableRegionID_);
			subRegion = (dataIndex != -1) ? textureDatas_.at(dataIndex).regionPtr : NULL;
			viewID = getViewID();
			linkMgr->link(mainRegion, subRegion, viewID);
		}
		

		// �I�����X�V
		selectRegionID_ = selectEnableRegionID_;
		updateSelect();
		updateSelectEnable();
		emit regionSelectEnabled();
		emit regionSelected();
	}
}


/*!
	@brief	�I�����̍X�V
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

	// �|���S���쐬
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

	// �|���S���쐬
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
	@brief	�q�b�g�e�X�g
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
	@brief	�����_�Ƃ̃q�b�g�e�X�g
*/
void EditViewBase::hitTest_FeaturePoints()
{
	moveEnableFeaturePointIndex_ = -1;
	//
	// �I��̈�ɑΉ��̈悪����ꍇ�����q�b�g�e�X�g���s��
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

	// �Q�ȏ�̑Ή��̈悪����΃q�b�g�e�X�g���s��
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

	// �X�N���[�����W�n�Ōv�Z����
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
	@brief �����_�̈ړ�
*/
void EditViewBase::movedFeaturePoint()
{
	int movedFeaturePointIndex = -1;
	//
	// �I��̈�ɑΉ��̈悪����ꍇ�����q�b�g�e�X�g���s��
	//

	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	RegionLinkData* selectLinkData = select->selectLinkData;
	if(!selectLinkData)
		return;

	ClosedRegion* selectRegion = select->selectRegion;
	ClosedRegion* thisSelectRegion = selectLinkData->getRegion(getViewID());

	if(!thisSelectRegion)
		return;

	// �Q�ȏ�̑Ή��̈悪����΃q�b�g�e�X�g���s��
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

	// �X�N���[�����W�n�Ōv�Z����
	float minLength = 0xffff;
	QVector2D mouse(mousePos_.x(), height() - mousePos_.y());
	QVector2D windowCenter(w / 2.0f, h / 2.0f);
	QVector2D dispCenter = windowCenter * dispScale_;

	// �ړ��\�ȋ�Ԃ́A�O��̓����_�ɋ��܂ꂽ���
	// ���̋�Ԃ̒��_�����Ɣ���
	const QVector<int>* featureIndices = thisSelectRegion->getFeaturePointIndices();
	int selectFeaturePointIndex = featureIndices->at(select->selectFeatureIndex);

	// �ʏ�̔���
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

		// �����_�f�[�^��������
		if(movedFeaturePointIndex != -1)
		{
			thisSelectRegion->setFeaturePoint(select->selectFeatureIndex, movedFeaturePointIndex);
			selectLinkData->createLines();
		}
	}
	// 0�Ԗڂ̓����_�𓮂������͋��E�s�N�Z���̏��Ԃ��������K�v������ 
	else
	{
		// ������͈͂�0-1�ƍŌ�̓����_-�Ō�̒��_�̂Q�̋��
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

		// �����_�f�[�^��������
		if(movedFeaturePointIndex != -1)
		{
			int bufSize = featureIndices->size();
			int* buf = new int[ bufSize ];
			for(int i = 0; i < featureIndices->size(); i++)
			{
				buf[i] = featureIndices->at(i);
			}

			// �n�_�̈ʒu��ς���
			thisSelectRegion->setBoundaryStartPoint(movedFeaturePointIndex);
			
			// �����_�f�[�^�ł��ꂽ�����C��
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
		// ������͈͂�0-1�ƍŌ�̓����_-�Ō�̒��_�̂Q�̋��
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

		// �����_�f�[�^��������
		if(movedFeaturePointIndex_ != -1)
		{
			int* buf = new int[ features->size() ];
			for(int i = 0; i < features->size(); i++)
			{
				int featurePointIndex = qFloor(features->at(i) * boundaries.size() + 0.5);
				buf[i] = featurePointIndex;
			}

			// �n�_�̈ʒu��ς���
			thisSelectRegion->setBoundaryStartPoint(movedFeaturePointIndex_);
			
			// �����_�f�[�^�ł��ꂽ�����C��
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
	@brief	�I��̈�̋��E��̓_�Ƃ̃q�b�g�e�X�g
*/
void EditViewBase::hitTest_Vertices()
{
	selectPointIndex_ = -1;

	//
	// �I��̈�ɑΉ��̈悪����ꍇ�����q�b�g�e�X�g���s��
	//

	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	RegionLinkData* selectLinkData = select->selectLinkData;
	if(!selectLinkData)
		return;

	ClosedRegion* selectRegion = select->selectRegion;
	ClosedRegion* thisSelectRegion = selectLinkData->getRegion(getViewID());

	if(!thisSelectRegion)
		return;

	// �Q�ȏ�̑Ή��̈悪����΃q�b�g�e�X�g���s��
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

	// �X�N���[�����W�n�Ōv�Z����
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
		// �e�N�X�`���擾
		RegionTextureData data;
		ClosedRegion* r = regions.at(i);
		int w, h;
		w = r->getRegionMap().getWidth();
		h = r->getRegionMap().getHeight();

		// �`��p�ɔ����̃}�b�v�쐬
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

		// �o�E���f�B���O�{�b�N�X�v�Z
		// �����Ő��K�����Ă���
		IntVec::ivec2 bBoxMax = r->getBboxMax();
		IntVec::ivec2 bBoxMin = r->getBboxMin();
		float bw = bBoxMax.x - bBoxMin.x;
		float bh = bBoxMax.y - bBoxMin.y;
		QRectF rect((bBoxMin.x / (float)w) - 0.5f , (bBoxMin.y / (float)h) - 0.5f, bw / (float)w, bh / (float)h);

		// �e��f�[�^�Z�b�g
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
	@brief	�|���S���쐬
*/
QPoint EditViewBase::makePolygon(ClosedRegion& r, QColor& outColor, QPolygon& outPolygon)
{
	// �̈�̃f�[�^�͍�����(0,0)�A�E�オ(w,h)�ƂȂ��Ă���̂ŁA
	// �o�E���f�B���O�{�b�N�X�̒��S��0�ƂȂ�悤�ɕϊ�����
	// �܂�Qt�̍��W�n��Y���W���t�ɂȂ�̂ŕϊ�����

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

	// �I��̈�|���S���̏����ʒu�Z�b�g
	float x = (r.getBboxMax().x + r.getBboxMin().x) / 2.0f;
	float y = h - ((r.getBboxMax().y + r.getBboxMin().y) / 2.0f) - 1;
	
	return QPoint(x, y);
}

/*!
	@brief	�|���S���N���A
*/
void EditViewBase::clearPolygon()
{
	selectPolygon_.clear();
	selectEnablePolygon_.clear();
}

/*!
	@brief	�����_�Z�b�g
*/
void EditViewBase::setFeaturePoint()
{
	if(selectPointIndex_ == -1)
		return;

	// �I�𒸓_�̊������v�Z
	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	RegionLinkData* selectLinkData = select->selectLinkData;
	if(!selectLinkData)
		return;

	ClosedRegion* selectRegion = select->selectRegion;
	ClosedRegion* thisSelectRegion = selectLinkData->getRegion(getViewID());

	if(!thisSelectRegion)
		return;

	// �Q�ȏ�̑Ή��̈悪����Γ����_���Z�b�g����
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

	// �I��̈�ɓ����_�Z�b�g
	int dataIndex = thisSelectRegion->addFeaturePoint(selectPointIndex_);

	// �Ή��̈�ɓ����_�Z�b�g
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
	
	// �I��̈�ɓ����_�Z�b�g
	int dataIndex = thisSelectRegion->addFeaturePoint(ratio);

	// �Ή��̈�ɓ����_�Z�b�g
	// �����͑O��̊����̒��Ԓl���Z�b�g����
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
	// ���C���쐬
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
	@brief	�Ή��̈悪�Ȃ��ꍇ�A�����ɗ̈���쐬
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

	// �����őI���\�ȗ̈�ɑΉ��t����ꂽ�̈�
	ClosedRegion* thisSelectEnableRegion = NULL;
	if(selectViewID != -1 && selectEnableLinkData != NULL)
	{
		thisSelectEnableRegion = selectEnableLinkData->getRegion(getViewID());
	}
	
	// �����őI�������̈�ɑΉ��t����ꂽ�̈�
	ClosedRegion* thisSelectRegion = NULL;
	if(selectViewID != -1 && selectLinkData != NULL)
	{
		thisSelectRegion = selectLinkData->getRegion(getViewID());
	}

	// �|���S���쐬
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
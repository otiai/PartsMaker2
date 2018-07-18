#include "ModifierView.h"
#include <QtGui>
#include "AnimeFrame.h"
#include "ObjectManager.h"
#include "Utility.h"
#include <QColor>

static float sRegionAlpha = 0.4f;
static QColor sScribbleColor = QColor(255, 128, 128, 255);

//--------------------------------------------------
/*!
	@brief	�R���X�g���N�^
*/
ModifierView::ModifierView(QWidget *parent) : selectRegion_(NULL), offscreenImage_(NULL), scribbleBufferImage_(NULL)
{
	// OpenGL���g���Ƃ���false�ɂ���
	setAutoFillBackground(false);

	// �}�E�X�{�^���������ĂȂ��Ă�MouseMove�C�x���g������悤�ɂȂ�
	setMouseTracking(true);

	// �f�t�H���g�E�B���h�E�T�C�Y�ݒ�
	resize(500, 500);
	setMinimumSize( width(), height() );
	setMaximumSize( width(), height() );

	// �L�[�{�[�h�Ƀt�H�[�J�X����
	setFocusPolicy(Qt::ClickFocus);

	dispScale_ = 1.0f;
	selectRegionTextureID_ = 0;

	isDispOtherRegions_ = true;

	setState(STATE_SELECT_NORMAL);

	polygonPos_.setX(0);
	polygonPos_.setY(0);

	polygonScale_ = QPointF(1.0f, 1.0f);

	selectEnableRegionID_ = -1;
}

//--------------------------------------------------
/*!
	@brief	�f�X�g���N�^
*/
ModifierView::~ModifierView()
{
	deleteTextures();
}

//--------------------------------------------------
/*!
	@brief	������
*/
void ModifierView::initializeGL()
{
	AnimeFrame* frame = ObjectManager::getInstance()->getSrcFrame();
	if(frame)
	{
		initImage();
	}
}

/*!
	@brief	�摜�ǂݍ��ݎ��̏�����
*/

void ModifierView::initImage()
{
	AnimeFrame* frame = ObjectManager::getInstance()->getSrcFrame();
	int w = frame->getIDMap().getWidth();
	int h = frame->getIDMap().getHeight();
	setMinimumSize( w, h );
	setMaximumSize( w, h );
	rebuild();
	resize(w, h);
}

//--------------------------------------------------
/*!
	@brief	�r���[�|�[�g�ݒ�
*/
void ModifierView::setupViewport(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, +1.0, -1.0, 1.0, 0.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
}
//--------------------------------------------------
/*!
	@brief	���T�C�Y
*/
void ModifierView::resizeGL(int width, int height)
{
	scribbleBuffer_.allocate( width, height );
	scribbleBuffer_.fill( IntVec::ubvec4(0,0,0,0) );

	createScribbleImage();

	glViewport(0, 0, width, height);
	glLoadIdentity();
}

//--------------------------------------------------
/*!
	@brief	�`��
*/
void ModifierView::paintEvent(QPaintEvent *event)
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

	// QPainter�ɂ��`��i2D�j
	QPainter painter(this);
	draw2D(&painter);
	painter.end();
}

//--------------------------------------------------
/*!
	@brief	QPainter�ɂ��`��i2D�j
*/
void ModifierView::draw2D(QPainter* painter)
{
	QString str = QString("STATE = %1").arg(getState());
	QString str2 = QString("ply X = %1, Y = %2").arg(polygonPos_.x()).arg(polygonPos_.y());
	painter->setPen(Qt::green);
	//painter->drawText( 30, 30, str );
	//painter->drawText( 30, 50, str2 );

	// �I��̈�|���S���`��
	drawRegionPolygon(*painter, regionPolygon_, polygonColor_, sSelectRegionEdgeColor, polygonPos_, true, polygonScale_);

	// �I���\�̈�|���S���`��
	drawRegionPolygon(*painter, selectEnablePolygon_, selectEnablePolygonColor_, sSelectEnableRegionEdgeColor, selectEnablePolygonPos_, true);

	// �X�N���u���`��
	if(getState() == STATE_FREEHAND_NORMAL ||
		getState() == STATE_COMBINE_NORMAL ||
		getState() == STATE_DIVIDE_NORMAL)
	{
		painter->drawImage(0, 0, *scribbleBufferImage_);
	}
}

//--------------------------------------------------
/*!
	@brief	�}�E�X�i�����j�C�x���g
*/
void ModifierView::mousePressEvent(QMouseEvent *event)
{
	/*
		event->button() == Qt::LeftButton
		�Ȃǂŉ����ꂽ�{�^���擾
		�}�E�X���W��event->x(), event->y()�ȂǂŎ擾
	*/
	switch(getState())
	{
	case STATE_SELECT_NORMAL:
		selectRegion();
		break;

	case STATE_TRANSLATION_NORMAL:
		if(event->button() == Qt::LeftButton)
		{
			mousePosOld_ = mousePos_;
			setState(STATE_TRANSLATION_SELECTED);
		}
		break;

	case STATE_TRANSLATION_SELECTED:
		break;

	case STATE_FREEHAND_NORMAL:
	case STATE_COMBINE_NORMAL:
	case STATE_DIVIDE_NORMAL:
		if(event->button() == Qt::LeftButton)
		{
			scribbleBrush_.paint( scribbleBuffer_, event->x(), height() - event->y() );
			updateScribbleImage();
		}
		break;

	default:
		break;
	}
}

//--------------------------------------------------
/*!
	@brief	�}�E�X�i�����j�C�x���g
*/
void ModifierView::mouseReleaseEvent(QMouseEvent *event)
{
	switch(getState())
	{
	case STATE_TRANSLATION_NORMAL:
		break;

	case STATE_TRANSLATION_SELECTED:
		if(event->button() == Qt::LeftButton)
		{
			translateRegion();
			setState(STATE_TRANSLATION_NORMAL);
		}
		break;

	case STATE_FREEHAND_NORMAL:
		if(event->button() == Qt::LeftButton)
		{
			scribbleBrush_.release();
			modifyRegion();
		}
		break;

	case STATE_COMBINE_NORMAL:
		if(event->button() == Qt::LeftButton)
		{
			scribbleBrush_.release();
			combineRegion();
		}
		break;

	case STATE_DIVIDE_NORMAL:
		if(event->button() == Qt::LeftButton)
		{
			scribbleBrush_.release();
			divideRegion();;
		}
		break;
	default:
		break;
	}
}

//--------------------------------------------------
/*!
	@brief	�}�E�X�ړ��C�x���g
*/
void ModifierView::mouseMoveEvent(QMouseEvent *event)
{
	mousePos_ = event->pos();

	switch(getState())
	{
	case STATE_SELECT_NORMAL:
		hitTest();
		makeSelectEnablePolygon();
		break;

	case STATE_TRANSLATION_NORMAL:
		break;

	case STATE_TRANSLATION_SELECTED:
		movePolygon();
		break;

	case STATE_FREEHAND_NORMAL:
	case STATE_COMBINE_NORMAL:
	case STATE_DIVIDE_NORMAL:
		if(scribbleBrush_.isUsed())
		{
			scribbleBrush_.paint( scribbleBuffer_, event->x(), height() - event->y() );
			updateScribbleImage();
		}
		break;
	default:
		break;
	}

	update();
}

//------------------------------------------------------
/*!
	@brief	�L�[�{�[�h�C�x���g
*/
void ModifierView::keyPressEvent(QKeyEvent *event)
{
	/*
		event->key() == Qt::Key_S
		�Ȃǂŉ����ꂽ�L�[���擾
	*/

	float offset = 0.1f;
	switch(getState())
	{
	case STATE_TRANSLATION_NORMAL:

		switch(event->key())
		{
		case Qt::Key_R:
			polygonScale_.setX(polygonScale_.x() + offset);
			break;

		case Qt::Key_T:
			polygonScale_.setX(polygonScale_.x() - offset);
			break;

		case Qt::Key_F:
			polygonScale_.setY(polygonScale_.y() + offset);
			break;

		case Qt::Key_G:
			polygonScale_.setY(polygonScale_.y() - offset);
			break;
		}
		update();
		break;

	case STATE_TRANSLATION_SELECTED:
		break;

	case STATE_FREEHAND_NORMAL:
		break;
	}
}

void ModifierView::makeTextures()
{
	makeCurrent();
	deleteTextures();

	ObjectManager* mgr = ObjectManager::getInstance();
	SelectRegionData* select = mgr->getSelectRegionData();

	AnimeFrame* frame = NULL;
	if(select->selectedViewID == VIEW_FRONT)
	{
		frame = mgr->getSrcFrame();
	}
	else if(select->selectedViewID == VIEW_SIDE_RIGHT)
	{
		frame = mgr->getDstFrame();
	}

	// �I�����Ȃ��Ƃ��͉������Ȃ�
	if(!frame)
		return;
	
	std::vector<ClosedRegion*> regions = frame->getRegions();

	for(int i = 0; i < regions.size(); i++)
	{
		// �e�N�X�`���擾
		RegionTextureData data;
		ClosedRegion* r = regions.at(i);
		int w, h;
		w = r->getRegionMap().getWidth();
		h = r->getRegionMap().getHeight();

		GLuint id;
		glGenTextures(1, &id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);	
		glBindTexture(GL_TEXTURE_2D, id );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, r->getRegionMap().getData());	
		glBindTexture(GL_TEXTURE_2D, 0);

		// �o�E���f�B���O�{�b�N�X�v�Z
		// �����Ő��K�����Ă���
		IntVec::ivec2 bBoxMax = r->getBboxMax();
		IntVec::ivec2 bBoxMin = r->getBboxMin();
		float bw = bBoxMax.x - bBoxMin.x;
		float bh = bBoxMax.y - bBoxMin.y;
		QRectF rect((bBoxMin.x / (float)w) - 0.5f , (bBoxMin.y / (float)h) - 0.5f, bw / (float)w, bh / (float)h);

		// �e��f�[�^�Z�b�g
		data.textureID = id;
		data.regionID = r->getID();
		data.regionPtr = r;
		data.boundingBox = rect;
		textureDatas_.append(data);

		// �I��̈�̃e�N�X�`��ID�͕ێ�
		int selectRegionID = (select->selectRegion) ? select->selectRegion->getID() : -1;
		if(r->getID() == selectRegionID)
		{
			selectRegionTextureID_ = id;
		}
	}

	// �I��̈�\���p�̃I�t�X�N���[���C���[�W�쐬
	int w = frame->getIDMap().getWidth();
	int h = frame->getIDMap().getHeight();
	offscreenImage_ = new QImage(QSize(w, h), QImage::Format_ARGB32);
}

void ModifierView::updateTextures()
{
	for(int i = 0; i < textureDatas_.size(); i++)
	{
		ClosedRegion* r = textureDatas_.at(i).regionPtr;
		glBindTexture(GL_TEXTURE_2D, textureDatas_.at(i).textureID );
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width(), height(), GL_RGBA, GL_UNSIGNED_BYTE, r->getRegionMap().getData() );
	}
}


void ModifierView::deleteTextures()
{
	makeCurrent();

	for(int i = 0; i < textureDatas_.size(); i++)
	{
		GLuint id = textureDatas_.at(i).textureID;
		glDeleteTextures(1, &id);
	}
	textureDatas_.clear();

	if(offscreenImage_)
	{
		delete offscreenImage_;
	}
	offscreenImage_ = NULL;
}

void ModifierView::closeEvent(QCloseEvent *event)
{
	emit closed();
}

void ModifierView::rebuild()
{
	polygonScale_ = QPointF(1.0f, 1.0f);
	deleteTextures();
	makeTextures();

	// �|���S���쐬
	makePolygon();

	update();
}

void ModifierView::createScribbleImage()
{
	deleteScribbleImage();
	Utility::convertImageRGBAu2QImage(&scribbleBuffer_, &scribbleBufferImage_);

	if(!scribbleBufferImage_)
	{
		qDebug("none");
	}
}

void ModifierView::updateScribbleImage()
{
	// ���t���[���������Ă���̂͏d�������H
	deleteScribbleImage();
	createScribbleImage();
}

void ModifierView::deleteScribbleImage()
{
	if(scribbleBufferImage_)
	{
		delete scribbleBufferImage_;
	}
	scribbleBufferImage_ = NULL;
}

/*!
	@brief	�̈�`��
*/
void ModifierView::drawRegion()
{
	// �I��̈�ȊO�𔼓����ŕ\��
	// �I��̈�͕K���őO�ʂɕ`�悵�����̂�draw2D�ŕ`�悷��

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	int selectRegionID = (select->selectRegion) ? select->selectRegion->getID() : -1;

	for(int i = 0; i < textureDatas_.size(); i++)
	{
		int regionID = textureDatas_.at(i).regionID;

		if(!isDispOtherRegions_ && regionID != selectRegionID)
			continue;

		if(regionID == selectRegionID)
			continue;

		if(regionID == selectEnableRegionID_)
			continue;

		// �������[�h�̂Ƃ��͎����Ɠ����F�̗̈�͕s�����\��
		float alpha = sRegionAlpha;
		if(getState() == STATE_COMBINE_NORMAL && select->selectRegion)
		{
			IntVec::ubvec4 selectColor = select->selectRegion->getRegionColor();
			IntVec::ubvec4 thisColor = textureDatas_.at(i).regionPtr->getRegionColor();
			alpha = (selectColor == thisColor) ? 1.0f : sRegionAlpha;
		}

		glPushMatrix();
		glColor4f(1.0f, 1.0f, 1.0f, alpha);
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

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}

/*!
	@brief	�̈�C��
*/
void ModifierView::modifyRegion()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);  // �����v

	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	ClosedRegion* selectRegion = select->selectRegion;
	if(selectRegion)
	{
		// �̈�C��
		selectRegion->modifyRegion(scribbleBuffer_);
		updateTextures();
		makePolygon();
		emit regionModified();
	}
	clearScribble();
	update();

	QApplication::restoreOverrideCursor();  // ���ɖ߂�

}

/*!
	@brief	�̈挋��
*/
void ModifierView::combineRegion()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);  // �����v

	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	ClosedRegion* selectRegion = select->selectRegion;
	if(selectRegion)
	{
		// �̈�C��
		ObjectManager::getInstance()->combineRegion(*selectRegion, scribbleBuffer_, select->selectedViewID);
		rebuild();
		makePolygon();
		emit regionModified();
	}
	clearScribble();
	update();

	QApplication::restoreOverrideCursor();  // ���ɖ߂�
}

/*!
	@brief	�̈敪��
*/

void ModifierView::divideRegion()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);  // �����v

	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	ClosedRegion* selectRegion = select->selectRegion;
	if(selectRegion)
	{
		// �̈�C��
		bool isDivided = ObjectManager::getInstance()->divideRegion(*selectRegion, scribbleBuffer_, select->selectedViewID);
		if(isDivided)
		{
			ObjectManager* mgr = ObjectManager::getInstance();
			ClosedRegion* newSelect = NULL;
			if(select->selectedViewID == VIEW_MAIN)
			{
				int size = mgr->getSrcFrame()->getRegions().size();
				newSelect = mgr->getSrcFrame()->getRegions().at(size - 1);
			}
			else
			{
				int size = mgr->getDstFrame()->getRegions().size();
				newSelect = mgr->getDstFrame()->getRegions().at(size - 1);
			}

			// �I��������
			select->selectRegion = newSelect;
			select->selectEnableRegion = newSelect;
			select->selectLinkData = newSelect->getRegionLinkData();
			select->selectEnableLinkData = newSelect->getRegionLinkData();
			rebuild();
			makePolygon();
			emit regionDivided();
		}
	}
	clearScribble();
	update();

	QApplication::restoreOverrideCursor();  // ���ɖ߂�
}

/*!
	@brief	�̈�ړ�
*/
void ModifierView::translateRegion()
{
	makePolygonTexture();
	emit regionModified();
}


/*!
	@brief	�|���S���쐬
*/
QPoint ModifierView::makePolygon(ClosedRegion& r, QColor& outColor, QPolygon& outPolygon)
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

	outColor.setRed(r.getRegionColor().r);
	outColor.setGreen(r.getRegionColor().g);
	outColor.setBlue(r.getRegionColor().b);
	outColor.setAlpha(255);

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
	@brief	�I���\�|���S���쐬
*/
void ModifierView::makeSelectEnablePolygon()
{
	selectEnablePolygon_.clear();
	int dataIndex = getRegionIndex(selectEnableRegionID_);
	if(dataIndex != -1)
	{
		ClosedRegion* selectEnableRegion = textureDatas_.at(dataIndex).regionPtr;
		selectEnablePolygonPos_ = makePolygon(*selectEnableRegion, selectEnablePolygonColor_, selectEnablePolygon_);
	}
}

/*!
	@brief	�I��̈�̃|���S���쐬
*/
void ModifierView::makePolygon()
{
	regionPolygon_.clear();
	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	ClosedRegion* selectRegion = select->selectRegion;
	if(selectRegion)
	{
		polygonPos_ = makePolygon(*selectRegion, polygonColor_, regionPolygon_);
	}
}

void ModifierView::makePolygonTexture()
{
	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	ClosedRegion* selectRegion = select->selectRegion;
	if(selectRegion)
	{
		// �I�t�X�N���[���ɕ`��
		offscreenImage_->fill(QColor(0,0,0,0));
		QPainter painter(offscreenImage_);
		drawRegionPolygon(painter, regionPolygon_, polygonColor_, sSelectRegionEdgeColor, polygonPos_, false, polygonScale_);

		// RegionMap�ƃe�N�X�`���̏�������
		selectRegion->replaceRegionMap(*offscreenImage_);
		RegionMap& map = selectRegion->getRegionMap();
		makeCurrent();
		glBindTexture(GL_TEXTURE_2D, selectRegionTextureID_ );
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width(), height(), GL_RGBA, GL_UNSIGNED_BYTE, map.getData() );
	}
}

void ModifierView::drawRegionPolygon(QPainter& painter, QPolygon& polygon, QColor& color, QColor& edgeColor, 
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
	QBrush brush(color, Qt::SolidPattern);
	painter.setPen(pen);
	painter.setBrush(brush);

	painter.save();
	painter.translate(pos);
	painter.scale(scale.x(), scale.y());
	painter.drawPolygon(polygon);
	painter.restore();
}

void ModifierView::movePolygon()
{
	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	ClosedRegion* selectRegion = select->selectRegion;
	if(selectRegion)
	{
		int w = selectRegion->getRegionMap().getWidth();
		int h = selectRegion->getRegionMap().getHeight();

		QPoint sub = mousePos_ - mousePosOld_;
		int bhw = regionPolygon_.boundingRect().width() / 2;
		int bhh = regionPolygon_.boundingRect().height() / 2;
		int tempX = polygonPos_.x() + sub.x();
		int tempY = polygonPos_.y() + sub.y();

		tempX = (tempX - bhw < 0) ? bhw : tempX;
		tempX = (tempX + bhw > w) ? w - bhw : tempX;
		tempY = (tempY - bhh < 0) ? bhh : tempY;
		tempY = (tempY + bhh > h) ? h - bhh : tempY;
			
		polygonPos_.setX(tempX);
		polygonPos_.setY(tempY);
		mousePosOld_ = mousePos_;
	}
}

void ModifierView::clearScribble()
{
	scribbleBuffer_.fill( IntVec::ubvec4(0,0,0,0) );
	updateScribbleImage();
}

void ModifierView::setFreeHandMode()
{
	clearScribble();
	scribbleBrush_.setColor(sScribbleColor.red(), sScribbleColor.green(), sScribbleColor.blue(), sScribbleColor.alpha());
	setState(STATE_FREEHAND_NORMAL);
	update();
}

void ModifierView::setTranslationMode()
{
	clearScribble();
	setState(STATE_TRANSLATION_NORMAL);
	update();
}

void ModifierView::setDivideMode()
{
	clearScribble();
	scribbleBrush_.setColor(sScribbleColor.red(), sScribbleColor.green(), sScribbleColor.blue(), sScribbleColor.alpha());
	setState(STATE_DIVIDE_NORMAL);
	update();
}

void ModifierView::setCombineMode()
{
	clearScribble();
	scribbleBrush_.setColor(sScribbleColor.red(), sScribbleColor.green(), sScribbleColor.blue(), sScribbleColor.alpha());
	setState(STATE_COMBINE_NORMAL);
	update();
}

void ModifierView::setSelectMode()
{
	clearScribble();
	setState(STATE_SELECT_NORMAL);
	update();
}

void ModifierView::hitTest()
{
	QVector<int> hitIndices;
	float x = ((float)mousePos_.x() / (float)width()) - 0.5f;
	float y = 0.5f - ((float)mousePos_.y() / (float)height());

	int px = (x/dispScale_ + 0.5) * width();
	int py = (y/dispScale_ + 0.5) * height();

	
	for(int i = 0; i < textureDatas_.size(); i++)
	{
		ClosedRegion* r = textureDatas_.at(i).regionPtr;
		
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

int ModifierView::getRegionIndex(int regionID)
{
	for(int i = 0; i < textureDatas_.size(); i++)
	{
		int id = textureDatas_.at(i).regionID;
		if(id == regionID)
		{
			return i;
		}
	}
	return -1;
}

void ModifierView::selectRegion()
{
	if(selectEnableRegionID_ == -1)
		return;

	int dataIndex = getRegionIndex(selectEnableRegionID_);
	selectRegion_ = textureDatas_.at(dataIndex).regionPtr;
	SelectRegionData* select = ObjectManager::getInstance()->getSelectRegionData();
	select->selectRegion = selectRegion_;
	selectRegionTextureID_ = textureDatas_.at(dataIndex).textureID;

	makePolygon();
	makePolygonTexture();
	makeSelectEnablePolygon();

	emit regionSelected();
}


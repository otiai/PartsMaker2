#include "EditWindow.h"
#include "EditViewSrc.h"
#include "EditViewDst.h"
#include <QtGui>
#include "ObjectManager.h"

EditWindow::EditWindow(QWidget *parent)
{
	this->setParent(parent);

	QHBoxLayout *hLayout = new QHBoxLayout;

	src_ = new EditViewSrc();
	dst_ = new EditViewDst();

	hLayout->addWidget(src_);
	hLayout->addWidget(dst_);

	// Set layout in QWidget
    dualWidget_ = new DualWidget();
    dualWidget_->setLayout(hLayout);

    // Set QWidget as the central layout of the main window
    setCentralWidget(dualWidget_);

	connect(src_, SIGNAL(regionSelectEnabled()), dst_, SLOT(rebuildPolygons()));
	connect(dst_, SIGNAL(regionSelectEnabled()), src_, SLOT(rebuildPolygons()));

	connect(src_, SIGNAL(regionSelected()), this, SLOT(changeSelectedRegion()));
	connect(dst_, SIGNAL(regionSelected()), this, SLOT(changeSelectedRegion()));

	connect(src_, SIGNAL(matchedRegionCreated()), dst_, SLOT(rebuildTextures()));
	connect(dst_, SIGNAL(matchedRegionCreated()), src_, SLOT(rebuildTextures()));

	connect(src_, SIGNAL(globalPosUpdated()), dst_, SLOT(update()));
	connect(dst_, SIGNAL(globalPosUpdated()), src_, SLOT(update()));
	connect(src_, SIGNAL(globalPosUpdated()), dualWidget_, SLOT(update()));
	connect(dst_, SIGNAL(globalPosUpdated()), dualWidget_, SLOT(update()));

	createActions();
	createToolBars();
}

EditWindow::~EditWindow()
{
}

void EditWindow::initImage()
{
	src_->initImage();
	dst_->initImage();

	src_->adjustSize();
	dst_->adjustSize();
	dualWidget_->adjustSize();
	this->adjustSize();
}

void EditWindow::createActions()
{
	// 領域対応付けモード
	setRegionMatchModeAct_ = new QAction(this);
	setRegionMatchModeAct_->setCheckable(true);
	setRegionMatchModeAct_->setIcon(QPixmap("../PartsMaker2/resources/icons/regionlink.png"));
	connect(setRegionMatchModeAct_, SIGNAL(triggered()), this, SLOT(setRegionMatchMode()));

	// 頂点対応付けモード
	setVertexMatchModeAct_ = new QAction(this);
	setVertexMatchModeAct_->setIcon(QPixmap("../PartsMaker2/resources/icons/vertexlink.png"));
	setVertexMatchModeAct_->setCheckable(true);
	connect(setVertexMatchModeAct_, SIGNAL(triggered()), this, SLOT(setVertexMatchMode()));
	
	// グループ
	modeActionGroup_ = new QActionGroup(this);
	modeActionGroup_->addAction(setRegionMatchModeAct_);
	modeActionGroup_->addAction(setVertexMatchModeAct_);
	setRegionMatchModeAct_->setChecked(true);


	// 修正ウィンドウオープン
	openModifierWindow_ = new QAction(this);
	openModifierWindow_->setIcon(QPixmap("../PartsMaker2/resources/icons/modifier.png"));
	connect(openModifierWindow_, SIGNAL(triggered()), this->parent(), SLOT(createModifierWindow()));
	
}

void EditWindow::createToolBars()
{
	modeToolBar_ = addToolBar(tr("edit mode"));
	modeToolBar_->addAction(setRegionMatchModeAct_);
	modeToolBar_->addAction(setVertexMatchModeAct_);

	shortcutToolBar_ = addToolBar(tr("shortcut"));
	shortcutToolBar_->addAction(openModifierWindow_);
}

void EditWindow::closeEvent(QCloseEvent *event)
{
	emit closed();
}

void EditWindow::changeSelectedRegion()
{
	emit selectedRegionChanged();
}

void EditWindow::modifyRegions()
{
	src_->rebuildTextures();
	dst_->rebuildTextures();
}

void EditWindow::divideRegions()
{
	src_->rebuildTextures();
	dst_->rebuildTextures();
	src_->rebuildPolygons();
	dst_->rebuildPolygons();
}

void EditWindow::setRegionMatchMode()
{
	ObjectManager::getInstance()->setEditMode(MODE_REGION_MATCH);
	src_->update();
	dst_->update();
}

void EditWindow::setVertexMatchMode()
{
	ObjectManager::getInstance()->setEditMode(MODE_FEATUREPOINT_EDIT);
	src_->update();
	dst_->update();
}

//=================================
void DualWidget::paintEvent(QPaintEvent *event)
{
	ObjectManager* mgr = ObjectManager::getInstance();
	if(mgr->isRegionLinking())
	{
		QPainter painter(this);
		QPoint start = mgr->getDragStartGlobalPos();
		QPoint end = mgr->getDragEndGlobalPos();
		painter.setPen(QPen(sLinkingLineColor));
		painter.drawLine(start, end);
	}
}
#include "ModifierWindow.h"
#include "ModifierView.h"
#include <QtGui>

ModifierWindow::ModifierWindow(QWidget *parent)
{
	view_ = new ModifierView();
	setCentralWidget(view_);
	connect(view_, SIGNAL(regionModified()), this, SLOT(noticeRegionModified()));
	connect(view_, SIGNAL(regionDivided()), this, SLOT(noticeRegionDivided()));


	createActions();
	createToolBars();
}

ModifierWindow::~ModifierWindow()
{
}

void ModifierWindow::initImage()
{
	view_->initImage();
	view_->adjustSize();
	this->adjustSize();
}

void ModifierWindow::createActions()
{
	// 選択モード
	setSelectModeAct_ = new QAction(this);
	setSelectModeAct_->setCheckable(true);
	setSelectModeAct_->setIcon(QPixmap("../PartsMaker2/resources/icons/select.png"));
	connect(setSelectModeAct_, SIGNAL(triggered()), view_, SLOT(setSelectMode()));

	// 移動モード
	setTranslationModeAct_ = new QAction(this);
	setTranslationModeAct_->setCheckable(true);
	setTranslationModeAct_->setIcon(QPixmap("../PartsMaker2/resources/icons/translation.png"));
	connect(setTranslationModeAct_, SIGNAL(triggered()), view_, SLOT(setTranslationMode()));

	// フリーハンドモード
	setFreeHandModeAct_ = new QAction(this);
	setFreeHandModeAct_->setIcon(QPixmap("../PartsMaker2/resources/icons/pen.png"));
	setFreeHandModeAct_->setCheckable(true);
	connect(setFreeHandModeAct_, SIGNAL(triggered()), view_, SLOT(setFreeHandMode()));

	// 分割モード
	setDivideModeAct_ = new QAction(this);
	setDivideModeAct_->setIcon(QPixmap("../PartsMaker2/resources/icons/divide.png"));
	setDivideModeAct_->setCheckable(true);
	connect(setDivideModeAct_, SIGNAL(triggered()), view_, SLOT(setDivideMode()));
	
	// 結合モード
	setCombineModeAct_ = new QAction(this);
	setCombineModeAct_->setIcon(QPixmap("../PartsMaker2/resources/icons/combine.png"));
	setCombineModeAct_->setCheckable(true);
	connect(setCombineModeAct_, SIGNAL(triggered()), view_, SLOT(setCombineMode()));

	// グループ
	modeActionGroup_ = new QActionGroup(this);
	modeActionGroup_->addAction(setSelectModeAct_);
	modeActionGroup_->addAction(setTranslationModeAct_);
	modeActionGroup_->addAction(setFreeHandModeAct_);
	modeActionGroup_->addAction(setDivideModeAct_);
	modeActionGroup_->addAction(setCombineModeAct_);
	setSelectModeAct_->setChecked(true);
}

void ModifierWindow::createToolBars()
{
	modeToolBar_ = addToolBar(tr("edit mode"));
	modeToolBar_->addAction(setSelectModeAct_);
	modeToolBar_->addAction(setTranslationModeAct_);
	modeToolBar_->addAction(setFreeHandModeAct_);
	modeToolBar_->addAction(setDivideModeAct_);
	modeToolBar_->addAction(setCombineModeAct_);
}

void ModifierWindow::closeEvent(QCloseEvent *event)
{
	emit closed();
}

void ModifierWindow::changeSelectedRegion()
{
	view_->rebuild();
}

void ModifierWindow::noticeRegionModified()
{
	emit regionModified();
}

void ModifierWindow::noticeRegionDivided()
{
	emit regionDivided();
}
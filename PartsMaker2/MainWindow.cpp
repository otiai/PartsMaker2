#include <QtGui>
#include "MainWindow.h"
#include "ObjectManager.h"
#include "DepthViewBase.h"
#include "EditWindow.h"
#include "ModifierWindow.h"
#include "Dialogs.h"


MainWindow::MainWindow() : depthView_(NULL), editView_(NULL), modifierWindow_(NULL), edgeSettingDialog_(NULL)
{
	ObjectManager::create();
	ObjectManager::getInstance()->initialize(this);

	mdiArea_ = new QMdiArea;
	mdiArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	mdiArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setCentralWidget(mdiArea_);

	createActions();
	createMenus();
	createEditView();
	createDepthView();
	//createModifierWindow(); // 修正ウィンドウは邪魔なので最初は出さない
	
	setWindowTitle(tr("2.5D Modeling"));
	setUnifiedTitleAndToolBarOnMac(true);
}

MainWindow::~MainWindow()
{
	ObjectManager::getInstance()->finalize();
	ObjectManager::destroy();
}

void MainWindow::closeEvent( QCloseEvent* event )
{
	qDebug("close");
}

void MainWindow::createActions()
{
	// 終了
	exitAct_ = new QAction(tr("Exit"), this);
	exitAct_->setShortcuts(QKeySequence::Quit);
	exitAct_->setStatusTip(tr("Exit the application"));
	connect(exitAct_, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

	// 画像読み込み
	loadImageAct_ = new QAction(tr("Load images"), this);
	loadImageAct_->setStatusTip(tr("Load image files"));
	connect(loadImageAct_, SIGNAL(triggered()), this, SLOT(openLoadImageDialog()));

	// 角度設定
	setRotaionAct_ = new QAction(tr("Rotation setting"), this);
	setRotaionAct_->setStatusTip(tr("Set rotations"));
	connect(setRotaionAct_, SIGNAL(triggered()), this, SLOT(openSettingDialog()));

	// エッジの太さ設定
	setEdgeWidthAct_ = new QAction(tr("Edge setting"), this);
	setEdgeWidthAct_->setStatusTip(tr("Set the width of edges"));
	connect(setEdgeWidthAct_, SIGNAL(triggered()), this, SLOT(openEdgeSettingDialog()));

	// エディットビュー
	editViewAct_ = new QAction(tr("Edit view"), this);
	editViewAct_->setStatusTip(tr("Open a edit view"));
	connect(editViewAct_, SIGNAL(triggered()), this, SLOT(createEditView()));

	// デプスビュー
	depthViewAct_ = new QAction(tr("Depth view"), this);
	depthViewAct_->setStatusTip(tr("Open a depth view"));
	connect(depthViewAct_, SIGNAL(triggered()), this, SLOT(createDepthView()));

	// 修正ビュー
	modifierViewAct_ = new QAction(tr("Modifier view"), this);
	modifierViewAct_->setStatusTip(tr("Open a modifier view"));
	connect(modifierViewAct_, SIGNAL(triggered()), this, SLOT(createModifierWindow()));

}

void MainWindow::createMenus()
{
	fileMenu_ = menuBar()->addMenu(tr("File"));
	fileMenu_->addAction(loadImageAct_);
	fileMenu_->addAction(exitAct_);

	windowMenu_ = menuBar()->addMenu(tr("Window"));
	windowMenu_->addAction(editViewAct_);
	windowMenu_->addAction(depthViewAct_);
	windowMenu_->addAction(modifierViewAct_);

	settingMenu_ = menuBar()->addMenu(tr("Setting"));
	settingMenu_->addAction(setRotaionAct_);
	settingMenu_->addAction(setEdgeWidthAct_);
}

void MainWindow::createEditView()
{
	if(!editView_)
	{
		editView_ = new EditWindow(this);
		editView_->setWindowTitle(tr("Edit Window"));
		mdiSubEditWin_ = mdiArea_->addSubWindow(editView_);
		editView_->show();
		connect(editView_, SIGNAL(closed()), this, SLOT(closeEditView()));

		if(modifierWindow_)
		{
			connect(editView_, SIGNAL(selectedRegionChanged()), modifierWindow_, SLOT(changeSelectedRegion()));
			connect(modifierWindow_, SIGNAL(regionModified()), editView_, SLOT(modifyRegions()));
			connect(modifierWindow_, SIGNAL(regionSelected()), editView_, SLOT(changeSelectedRegion()));
			connect(modifierWindow_, SIGNAL(regionDivided()), editView_, SLOT(divideRegions()));
		}
	}
	else
	{
		editView_->setFocus(Qt::MouseFocusReason);
	}
}

void MainWindow::closeEditView()
{
	editView_ = NULL;
}

void MainWindow::createDepthView()
{
	if(!depthView_)
	{
		depthView_ = new DepthViewBase;
		depthView_->setWindowTitle(tr("Depth View"));
		mdiSubDepthWin_ = mdiArea_->addSubWindow(depthView_);
		depthView_->show();
		connect(depthView_, SIGNAL(closed()), this, SLOT(closeDepthView()));

		if(modifierWindow_)
		{
			connect(modifierWindow_, SIGNAL(regionModified()), depthView_, SLOT(update()));
		}
	}
	else
	{
		depthView_->setFocus(Qt::MouseFocusReason);
	}
}

void MainWindow::closeDepthView()
{
	depthView_ = NULL;
}

void MainWindow::createModifierWindow()
{
	if(!modifierWindow_)
	{
		modifierWindow_ = new ModifierWindow;
		modifierWindow_->setWindowTitle(tr("Modifier View"));
		mdiSubModifierWin_ = mdiArea_->addSubWindow(modifierWindow_);
		modifierWindow_->show();
		connect(modifierWindow_, SIGNAL(closed()), this, SLOT(closeModifierWindow()));

		if(editView_)
		{
			connect(editView_, SIGNAL(selectedRegionChanged()), modifierWindow_, SLOT(changeSelectedRegion()));
			connect(modifierWindow_, SIGNAL(regionModified()), editView_, SLOT(modifyRegions()));
			connect(modifierWindow_, SIGNAL(regionSelected()), editView_, SLOT(changeSelectedRegion()));
			connect(modifierWindow_, SIGNAL(regionDivided()), editView_, SLOT(divideRegions()));
		}

		if(depthView_)
		{
			connect(modifierWindow_, SIGNAL(regionModified()), depthView_, SLOT(update()));
		}
	}
	else
	{
		modifierWindow_->setFocus(Qt::MouseFocusReason);
	}
}

void MainWindow::closeModifierWindow()
{
	modifierWindow_ = NULL;
}

void MainWindow::noticeLinkDataUpdated()
{
	if(depthView_)
	{
		depthView_->rebuild();
	}
}

void MainWindow::openLoadImageDialog()
{
	ImageFileLoadDialog* dlg = new ImageFileLoadDialog(this);
	if(dlg->exec() == QDialog::Accepted)
	{
		QApplication::setOverrideCursor(Qt::WaitCursor);  // 砂時計
		ObjectManager::getInstance()->loadImageFiles();

		if(depthView_)
		{
			depthView_->initImage();
			mdiSubDepthWin_->adjustSize();
		}

		if(editView_)
		{
			editView_->initImage();
			mdiSubEditWin_->adjustSize();
		}

		if(modifierWindow_)
		{
			modifierWindow_->initImage();
			mdiSubModifierWin_->adjustSize();
		}

		QApplication::restoreOverrideCursor();  // 元に戻す
	}
	delete dlg;
}

void MainWindow::openSettingDialog()
{
	RotationSettingDialog* dlg = new RotationSettingDialog(this);
	if(dlg->exec() == QDialog::Accepted)
	{
		QVector2D srcRot = dlg->getSrcRot();
		QVector2D dstRot = dlg->getDstRot();
		ObjectManager* mgr = ObjectManager::getInstance();
		mgr->setSrcRotation(srcRot);
		mgr->setDstRotation(dstRot);
		mgr->reCalcDepth();
	}
	delete dlg;
}

void MainWindow::openEdgeSettingDialog()
{

	if(!edgeSettingDialog_)
	{
		edgeSettingDialog_ = new EdgeSettingDialog(this);
		connect(edgeSettingDialog_, SIGNAL(closed()), this, SLOT(closeEdgeSettingDialog()));
	}

	if(edgeSettingDialog_->isHidden() )
	{
		edgeSettingDialog_->show();
	}
	else
	{
		edgeSettingDialog_->activateWindow();
	}
}

void MainWindow::closeEdgeSettingDialog()
{
	edgeSettingDialog_ = NULL;
}


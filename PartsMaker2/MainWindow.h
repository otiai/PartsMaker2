#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QtWidgets/QMainWindow>

class QMdiSubWindow;
class QAction;
class QMenu;
class QMdiArea;
class DepthViewBase;
class EditWindow;
class ModifierWindow;
class EdgeSettingDialog;

class MainWindow : public QMainWindow
{
	Q_OBJECT

signals:

public slots:
	void createEditView();
	void closeEditView();
	void createDepthView();
	void closeDepthView();
	void createModifierWindow();
	void closeModifierWindow();
	void openLoadImageDialog();
	void openSettingDialog();
	void openEdgeSettingDialog();
	void closeEdgeSettingDialog();

public:
	MainWindow();
	~MainWindow();
	void noticeLinkDataUpdated();
	void noticeRegionSelected();

	DepthViewBase* getDepthView(){ return depthView_; }

protected:
	void closeEvent( QCloseEvent* event );

private:
	void createMenus();
	void createActions();

private:
	QMdiArea*		mdiArea_;
	QMenu*			fileMenu_;
	QMenu*			windowMenu_;
	QMenu*			settingMenu_;

	QAction*		exitAct_;
	QAction*		loadImageAct_;
	QAction*		setRotaionAct_;
	QAction*		setEdgeWidthAct_;

	QAction*		editViewAct_;
	QAction*		depthViewAct_;
	QAction*		modifierViewAct_;

	DepthViewBase*	depthView_;
	EditWindow*		editView_;
	ModifierWindow*	modifierWindow_;

	QMdiSubWindow*	mdiSubDepthWin_;
	QMdiSubWindow*	mdiSubEditWin_;
	QMdiSubWindow*	mdiSubModifierWin_;

	EdgeSettingDialog*	edgeSettingDialog_;
};

#endif // MAIN_WINDOW_H
#ifndef EDIT_WINDOW_H
#define EDIT_WINDOW_H

#include <QMainWindow>
#include <QWidget>

class DualWidget;
class QToolBar;
class QActionGroup;
class QAction;
class EditViewSrc;
class EditViewDst;
class EditWindow : public QMainWindow
{
	Q_OBJECT
signals:
	void closed();
	void selectedRegionChanged();

public slots:
	void changeSelectedRegion();
	void modifyRegions();
	void setRegionMatchMode();
	void setVertexMatchMode();
	void divideRegions();

public:
	EditWindow(QWidget *parent = 0);
	~EditWindow();

	void initImage();

protected:
	void createActions();
	void createToolBars();
	void closeEvent(QCloseEvent *event);


protected:
	EditViewSrc*	src_;
	EditViewDst*	dst_;

	QActionGroup*	modeActionGroup_;
	QAction*		setRegionMatchModeAct_;
	QAction*		setVertexMatchModeAct_;

	QAction*		openModifierWindow_;
	QToolBar*		modeToolBar_;
	QToolBar*		shortcutToolBar_;

	DualWidget*		dualWidget_;
};

class DualWidget : public QWidget
{
	Q_OBJECT

protected:
	void paintEvent(QPaintEvent *event);
};

#endif // EDIT_WINDOW_H
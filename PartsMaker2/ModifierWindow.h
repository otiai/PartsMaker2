#ifndef MODIFIER_WINDOW_H
#define MODIFIER_WINDOW_H


#include <QMainWindow>

class QActionGroup;
class QAction;
class QToolBar;
class ModifierView;
class ModifierWindow : public QMainWindow
{
	Q_OBJECT
signals:
	void closed();
	void regionModified();
	void regionDivided();

public slots:
	void changeSelectedRegion();
	void noticeRegionModified();
	void noticeRegionDivided();

public:
	ModifierWindow(QWidget *parent = 0);
	~ModifierWindow();

	void initImage();

protected:
	void createActions();
	void createToolBars();
	void closeEvent(QCloseEvent *event);

private:
	ModifierView*	view_;
	QActionGroup*	modeActionGroup_;
	QAction*		setFreeHandModeAct_;
	QAction*		setTranslationModeAct_;
	QAction*		setDivideModeAct_;
	QAction*		setCombineModeAct_;
	QAction*		setSelectModeAct_;

	QToolBar*		modeToolBar_;
};

#endif // MODIFIER_WINDOW_H
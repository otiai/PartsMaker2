#ifndef DIALOGS_H
#define DIALOGS_H

#include <QDialog>
#include <QVector2D>

class QLabel;
class QPushButton;
class QDoubleSpinBox;
class QSpinBox;
class QVBoxLayout;
class QGroupBox;

class VerticalBox : public QObject
{
	Q_OBJECT

public slots:
	void setFileName();
	void setRotX(double d);
	void setRotY(double d);

public:
	VerticalBox(const char* viewName, QWidget* parent, int viewID);

public:
	QWidget*		parent;
	QGroupBox*		groupBox;
	QVBoxLayout*	layout;
	QLabel*			fileNameEdit;
	QPushButton*	referenceButton;
	QDoubleSpinBox*	rotX;
	QDoubleSpinBox*	rotY;
	int				viewID;
};

class ImageFileLoadDialog : public QDialog
{
	Q_OBJECT

public:
	ImageFileLoadDialog( QWidget * parent = 0);
	~ImageFileLoadDialog();

protected:
	VerticalBox* createUI(const char* viewLabel);

private:
	VerticalBox*	srcInfo_;
	VerticalBox*	dstInfo_;
};

class RotationSettingDialog : public QDialog
{
	Q_OBJECT
	
public:
	RotationSettingDialog( QWidget * parent = 0 );
	~RotationSettingDialog();
	QVector2D getSrcRot();
	QVector2D getDstRot();

private:
	QDoubleSpinBox*	srcRotX_;
	QDoubleSpinBox*	srcRotY_;

	QDoubleSpinBox*	dstRotX_;
	QDoubleSpinBox*	dstRotY_;
};

class EdgeSettingDialog : public QDialog
{
	Q_OBJECT

signals:
	void closed();

public slots:
	void setWidth(int w);

public:
	EdgeSettingDialog(QWidget * parent = 0);
	~EdgeSettingDialog();

	void closeEvent(QCloseEvent *event);

private:
	QSpinBox*	penWidth_;
};

#endif // DIALOGS_H
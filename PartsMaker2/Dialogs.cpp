#include "Dialogs.h"
#include <QtGui>
#include "RegionMatchHandler.h"
#include "ObjectManager.h"

ImageFileLoadDialog::ImageFileLoadDialog(QWidget* parent) : QDialog(parent), srcInfo_(NULL), dstInfo_(NULL)
{
	srcInfo_ = new VerticalBox("Left Image:", this, VIEW_MAIN);
	dstInfo_ = new VerticalBox("Right Image", this, VIEW_SIDE_RIGHT);

	// 初期値セット
	const QVector2D srcRot = ObjectManager::getInstance()->getSrcRotation();
	const QVector2D dstRot = ObjectManager::getInstance()->getDstRotation();
	srcInfo_->rotX->setValue(srcRot.x());
	srcInfo_->rotY->setValue(srcRot.y());
	dstInfo_->rotX->setValue(dstRot.x());
	dstInfo_->rotY->setValue(dstRot.y());
	
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                      | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout *vLayout = new QVBoxLayout;
	vLayout->addWidget(srcInfo_->groupBox);
	vLayout->addWidget(dstInfo_->groupBox);
	vLayout->addWidget(buttonBox);
	setLayout(vLayout);
}

ImageFileLoadDialog::~ImageFileLoadDialog()
{
	if(srcInfo_)
	{
		delete srcInfo_;
	}
	srcInfo_ = NULL;

	if(dstInfo_)
	{
		delete dstInfo_;
	}
	dstInfo_ = NULL;
}

VerticalBox::VerticalBox(const char* viewName, QWidget* p, int id)
{
	viewID = id;
	parent = p;
	layout = new QVBoxLayout;
	QHBoxLayout *hLayout_file = new QHBoxLayout;
	QHBoxLayout *hLayout_rot = new QHBoxLayout;

	fileNameEdit = new QLabel;
	fileNameEdit->setFrameStyle(QFrame::Sunken | QFrame::Panel);
	referenceButton = new QPushButton(tr("ref..."));
	rotX = new QDoubleSpinBox;
	rotX->setMinimum(-90.0);
	rotX->setMaximum(90.0);

	rotY = new QDoubleSpinBox;
	rotY->setMinimum(-180.0);
	rotY->setMaximum(180.0);

	hLayout_file->addWidget(fileNameEdit);
	hLayout_file->addWidget(referenceButton);

	hLayout_rot->addWidget(new QLabel(tr("X rotaion:")));
	hLayout_rot->addWidget(rotX);
	hLayout_rot->addWidget(new QLabel(tr("Y rotaion:")));
	hLayout_rot->addWidget(rotY);
	hLayout_rot->addStretch();

	layout->addLayout(hLayout_file);
	layout->addLayout(hLayout_rot);

	groupBox = new QGroupBox(viewName);
	groupBox->setLayout(layout);

	connect(referenceButton, SIGNAL(clicked()), this, SLOT(setFileName()));
	connect(rotX, SIGNAL(valueChanged(double)), this, SLOT(setRotX(double)));
	connect(rotY, SIGNAL(valueChanged(double)), this, SLOT(setRotY(double)));
}

/*!
	@brief	ファイル読み込みダイアログ
*/
void VerticalBox::setFileName()
{
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(parent,
                                 tr("Open an image file"),
                                 "../PartsMaker2/resources/",
                                 tr("Image Files (*);;PNG Files (*.png)"),
								 &selectedFilter);
     
	ObjectManager* mgr = ObjectManager::getInstance();
	if (!fileName.isEmpty())
	{
		fileNameEdit->setText(fileName);
	}

	// ファイル名は空でもセット
	if(viewID == VIEW_MAIN)
	{
		mgr->setSrcImageFileName(fileName);
	}
	else
	{
		mgr->setDstImageFileName(fileName);
	}
}

void VerticalBox::setRotX(double d)
{
	ObjectManager* mgr = ObjectManager::getInstance();
	if(viewID == VIEW_MAIN)
	{
		QVector2D rot = QVector2D(mgr->getSrcRotation());
		rot.setX(d);
		mgr->setSrcRotation(rot);
	}
	else
	{
		QVector2D rot = QVector2D(mgr->getDstRotation());
		rot.setX(d);
		mgr->setDstRotation(rot);
	}
}

void VerticalBox::setRotY(double d)
{
	ObjectManager* mgr = ObjectManager::getInstance();
	if(viewID == VIEW_MAIN)
	{
		QVector2D rot = QVector2D(mgr->getSrcRotation());
		rot.setY(d);
		mgr->setSrcRotation(rot);
	}
	else
	{
		QVector2D rot = QVector2D(mgr->getDstRotation());
		rot.setY(d);
		mgr->setDstRotation(rot);
	}
}

//================================
// RotationSettingDialog
//================================
RotationSettingDialog::RotationSettingDialog(QWidget* parent) : QDialog(parent)
{
	//src
	srcRotX_ = new QDoubleSpinBox;
	srcRotX_->setMinimum(-90.0);
	srcRotX_->setMaximum(90.0);

	srcRotY_ = new QDoubleSpinBox;
	srcRotY_->setMinimum(-180.0);
	srcRotY_->setMaximum(180.0);

	QFormLayout *srcLayout = new QFormLayout;
	srcLayout->addRow(new QLabel(tr("X rotation:")), srcRotX_);
	srcLayout->addRow(new QLabel(tr("Y rotation:")), srcRotY_);
	QGroupBox* srcGroup = new QGroupBox(tr("Left Image:"));
	srcGroup->setLayout(srcLayout);

	// dst
	dstRotX_ = new QDoubleSpinBox;
	dstRotX_->setMinimum(-90.0);
	dstRotX_->setMaximum(90.0);

	dstRotY_ = new QDoubleSpinBox;
	dstRotY_->setMinimum(-180.0);
	dstRotY_->setMaximum(180.0);
	dstRotY_->setValue(45);


	// 初期値セット
	const QVector2D srcRot = ObjectManager::getInstance()->getSrcRotation();
	const QVector2D dstRot = ObjectManager::getInstance()->getDstRotation();
	srcRotX_->setValue(srcRot.x());
	srcRotY_->setValue(srcRot.y());
	dstRotX_->setValue(dstRot.x());
	dstRotY_->setValue(dstRot.y());

	QFormLayout *dstLayout = new QFormLayout;
	dstLayout->addRow(new QLabel(tr("X rotation:")), dstRotX_);
	dstLayout->addRow(new QLabel(tr("Y rotation:")), dstRotY_);
	QGroupBox* dstGroup = new QGroupBox(tr("Reight Image:"));
	dstGroup->setLayout(dstLayout);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                      | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout *vLayout = new QVBoxLayout;
	vLayout->addWidget(srcGroup);
	vLayout->addWidget(dstGroup);
	vLayout->addWidget(buttonBox);
	setLayout(vLayout);
//	setLayout(vLayout);
}

RotationSettingDialog::~RotationSettingDialog()
{
}

QVector2D RotationSettingDialog::getSrcRot()
{
	QVector2D rot(srcRotX_->value(), srcRotY_->value());
	return rot;
}

QVector2D RotationSettingDialog::getDstRot()
{
	QVector2D rot(dstRotX_->value(), dstRotY_->value());
	return rot;
}

//================================
// EdgeSettingDialog
//================================
EdgeSettingDialog::EdgeSettingDialog(QWidget* parent) : QDialog(parent)
{
	//src
	penWidth_ = new QSpinBox;
	penWidth_->setMinimum(0);
	penWidth_->setMaximum(99.0);

	// 初期値セット
	const int width = ObjectManager::getInstance()->getEdgeWidth();
	penWidth_->setValue(width);

    connect(penWidth_, SIGNAL(valueChanged(int)), this, SLOT(setWidth(int)));
 
	QVBoxLayout *vLayout = new QVBoxLayout;
	vLayout->addWidget(new QLabel(tr("edge width:")));
	vLayout->addWidget(penWidth_);
	setLayout(vLayout);
}

EdgeSettingDialog::~EdgeSettingDialog()
{
}

void EdgeSettingDialog::setWidth(int w)
{
	ObjectManager::getInstance()->changeEdgeWidth(w);
}

void EdgeSettingDialog::closeEvent(QCloseEvent *event)
{
	emit closed();
}
/********************************************************************************
** Form generated from reading UI file 'partsmaker2.ui'
**
** Created by: Qt User Interface Compiler version 5.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PARTSMAKER2_H
#define UI_PARTSMAKER2_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PartsMaker2Class
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *PartsMaker2Class)
    {
        if (PartsMaker2Class->objectName().isEmpty())
            PartsMaker2Class->setObjectName(QStringLiteral("PartsMaker2Class"));
        PartsMaker2Class->resize(600, 400);
        menuBar = new QMenuBar(PartsMaker2Class);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        PartsMaker2Class->setMenuBar(menuBar);
        mainToolBar = new QToolBar(PartsMaker2Class);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        PartsMaker2Class->addToolBar(mainToolBar);
        centralWidget = new QWidget(PartsMaker2Class);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        PartsMaker2Class->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(PartsMaker2Class);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        PartsMaker2Class->setStatusBar(statusBar);

        retranslateUi(PartsMaker2Class);

        QMetaObject::connectSlotsByName(PartsMaker2Class);
    } // setupUi

    void retranslateUi(QMainWindow *PartsMaker2Class)
    {
        PartsMaker2Class->setWindowTitle(QApplication::translate("PartsMaker2Class", "PartsMaker2", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PartsMaker2Class: public Ui_PartsMaker2Class {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PARTSMAKER2_H

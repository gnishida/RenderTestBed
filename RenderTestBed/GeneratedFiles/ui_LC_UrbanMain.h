/********************************************************************************
** Form generated from reading UI file 'LC_UrbanMain.ui'
**
** Created: Fri Dec 11 20:13:12 2015
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LC_URBANMAIN_H
#define UI_LC_URBANMAIN_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDockWidget>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QSpacerItem>
#include <QtGui/QStatusBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LCUrbanMain
{
public:
    QWidget *centralwidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;
    QDockWidget *dockWidget;
    QWidget *dockWidgetContents;
    QGridLayout *gridLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_2;
    QDoubleSpinBox *ssaoRadiusSpinBox;
    QLabel *label;
    QLabel *label_2;
    QDoubleSpinBox *ssaoKernelSizeSpinBox;
    QLabel *label_3;
    QDoubleSpinBox *ssaoPowerSpinBox;
    QCheckBox *ssaoEnabledCheckBox;
    QSpacerItem *verticalSpacer;

    void setupUi(QMainWindow *LCUrbanMain)
    {
        if (LCUrbanMain->objectName().isEmpty())
            LCUrbanMain->setObjectName(QString::fromUtf8("LCUrbanMain"));
        LCUrbanMain->resize(800, 600);
        centralwidget = new QWidget(LCUrbanMain);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        LCUrbanMain->setCentralWidget(centralwidget);
        menubar = new QMenuBar(LCUrbanMain);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 21));
        LCUrbanMain->setMenuBar(menubar);
        statusbar = new QStatusBar(LCUrbanMain);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        LCUrbanMain->setStatusBar(statusbar);
        dockWidget = new QDockWidget(LCUrbanMain);
        dockWidget->setObjectName(QString::fromUtf8("dockWidget"));
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        gridLayout = new QGridLayout(dockWidgetContents);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        groupBox = new QGroupBox(dockWidgetContents);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        ssaoRadiusSpinBox = new QDoubleSpinBox(groupBox);
        ssaoRadiusSpinBox->setObjectName(QString::fromUtf8("ssaoRadiusSpinBox"));
        ssaoRadiusSpinBox->setDecimals(1);
        ssaoRadiusSpinBox->setMinimum(1);
        ssaoRadiusSpinBox->setValue(17);

        gridLayout_2->addWidget(ssaoRadiusSpinBox, 2, 1, 1, 1);

        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout_2->addWidget(label, 1, 0, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout_2->addWidget(label_2, 2, 0, 1, 1);

        ssaoKernelSizeSpinBox = new QDoubleSpinBox(groupBox);
        ssaoKernelSizeSpinBox->setObjectName(QString::fromUtf8("ssaoKernelSizeSpinBox"));
        ssaoKernelSizeSpinBox->setDecimals(0);
        ssaoKernelSizeSpinBox->setMinimum(1);
        ssaoKernelSizeSpinBox->setValue(16);

        gridLayout_2->addWidget(ssaoKernelSizeSpinBox, 1, 1, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout_2->addWidget(label_3, 3, 0, 1, 1);

        ssaoPowerSpinBox = new QDoubleSpinBox(groupBox);
        ssaoPowerSpinBox->setObjectName(QString::fromUtf8("ssaoPowerSpinBox"));
        ssaoPowerSpinBox->setDecimals(1);
        ssaoPowerSpinBox->setMinimum(0.1);
        ssaoPowerSpinBox->setValue(2);

        gridLayout_2->addWidget(ssaoPowerSpinBox, 3, 1, 1, 1);

        ssaoEnabledCheckBox = new QCheckBox(groupBox);
        ssaoEnabledCheckBox->setObjectName(QString::fromUtf8("ssaoEnabledCheckBox"));
        ssaoEnabledCheckBox->setChecked(true);

        gridLayout_2->addWidget(ssaoEnabledCheckBox, 0, 0, 1, 2);


        gridLayout->addWidget(groupBox, 0, 0, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 1, 0, 1, 1);

        dockWidget->setWidget(dockWidgetContents);
        LCUrbanMain->addDockWidget(static_cast<Qt::DockWidgetArea>(2), dockWidget);

        retranslateUi(LCUrbanMain);

        QMetaObject::connectSlotsByName(LCUrbanMain);
    } // setupUi

    void retranslateUi(QMainWindow *LCUrbanMain)
    {
        LCUrbanMain->setWindowTitle(QApplication::translate("LCUrbanMain", "LC_UrbanMain", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("LCUrbanMain", "SSAO", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("LCUrbanMain", "Kernel Size", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("LCUrbanMain", "Radius", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("LCUrbanMain", "Power", 0, QApplication::UnicodeUTF8));
        ssaoEnabledCheckBox->setText(QApplication::translate("LCUrbanMain", "SSAO Enabled", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class LCUrbanMain: public Ui_LCUrbanMain {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LC_URBANMAIN_H

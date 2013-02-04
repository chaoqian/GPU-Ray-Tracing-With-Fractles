/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created: Mon Dec 17 01:29:27 2012
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QSlider>
#include <QtGui/QStatusBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "GLWidget.hpp"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *exit_action;
    QAction *about_action;
    QAction *raytrace_cpu_action;
    QAction *raytrace_gpu_action;
    QAction *open_action;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QFrame *params_frame;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *fractal_group;
    QHBoxLayout *horizontalLayout;
    QSlider *fractal_x_slider;
    QSlider *fractal_y_slider;
    QSlider *fractal_z_slider;
    QSlider *fractal_w_slider;
    GLWidget *canvas_glwidget;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuRaytrace;
    QMenu *menuHelp;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(800, 600);
        exit_action = new QAction(MainWindow);
        exit_action->setObjectName(QString::fromUtf8("exit_action"));
        about_action = new QAction(MainWindow);
        about_action->setObjectName(QString::fromUtf8("about_action"));
        raytrace_cpu_action = new QAction(MainWindow);
        raytrace_cpu_action->setObjectName(QString::fromUtf8("raytrace_cpu_action"));
        raytrace_cpu_action->setCheckable(true);
        raytrace_gpu_action = new QAction(MainWindow);
        raytrace_gpu_action->setObjectName(QString::fromUtf8("raytrace_gpu_action"));
        raytrace_gpu_action->setCheckable(true);
        open_action = new QAction(MainWindow);
        open_action->setObjectName(QString::fromUtf8("open_action"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        params_frame = new QFrame(centralwidget);
        params_frame->setObjectName(QString::fromUtf8("params_frame"));
        params_frame->setMaximumSize(QSize(16777215, 80));
        params_frame->setFrameShape(QFrame::StyledPanel);
        params_frame->setFrameShadow(QFrame::Raised);
        verticalLayout_2 = new QVBoxLayout(params_frame);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        fractal_group = new QGroupBox(params_frame);
        fractal_group->setObjectName(QString::fromUtf8("fractal_group"));
        horizontalLayout = new QHBoxLayout(fractal_group);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        fractal_x_slider = new QSlider(fractal_group);
        fractal_x_slider->setObjectName(QString::fromUtf8("fractal_x_slider"));
        fractal_x_slider->setOrientation(Qt::Horizontal);

        horizontalLayout->addWidget(fractal_x_slider);

        fractal_y_slider = new QSlider(fractal_group);
        fractal_y_slider->setObjectName(QString::fromUtf8("fractal_y_slider"));
        fractal_y_slider->setOrientation(Qt::Horizontal);

        horizontalLayout->addWidget(fractal_y_slider);

        fractal_z_slider = new QSlider(fractal_group);
        fractal_z_slider->setObjectName(QString::fromUtf8("fractal_z_slider"));
        fractal_z_slider->setOrientation(Qt::Horizontal);

        horizontalLayout->addWidget(fractal_z_slider);

        fractal_w_slider = new QSlider(fractal_group);
        fractal_w_slider->setObjectName(QString::fromUtf8("fractal_w_slider"));
        fractal_w_slider->setOrientation(Qt::Horizontal);

        horizontalLayout->addWidget(fractal_w_slider);


        verticalLayout_2->addWidget(fractal_group);


        verticalLayout->addWidget(params_frame);

        canvas_glwidget = new GLWidget(centralwidget);
        canvas_glwidget->setObjectName(QString::fromUtf8("canvas_glwidget"));

        verticalLayout->addWidget(canvas_glwidget);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 21));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuRaytrace = new QMenu(menubar);
        menuRaytrace->setObjectName(QString::fromUtf8("menuRaytrace"));
        menuHelp = new QMenu(menubar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuRaytrace->menuAction());
        menubar->addAction(menuHelp->menuAction());
        menuFile->addAction(open_action);
        menuFile->addAction(exit_action);
        menuRaytrace->addAction(raytrace_cpu_action);
        menuRaytrace->addAction(raytrace_gpu_action);
        menuHelp->addAction(about_action);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        exit_action->setText(QApplication::translate("MainWindow", "Exit", 0, QApplication::UnicodeUTF8));
        about_action->setText(QApplication::translate("MainWindow", "About....", 0, QApplication::UnicodeUTF8));
        raytrace_cpu_action->setText(QApplication::translate("MainWindow", "Use CPU", 0, QApplication::UnicodeUTF8));
        raytrace_gpu_action->setText(QApplication::translate("MainWindow", "Use GPU", 0, QApplication::UnicodeUTF8));
        open_action->setText(QApplication::translate("MainWindow", "Open...", 0, QApplication::UnicodeUTF8));
        fractal_group->setTitle(QApplication::translate("MainWindow", "Fractal Parameters", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0, QApplication::UnicodeUTF8));
        menuRaytrace->setTitle(QApplication::translate("MainWindow", "Raytrace", 0, QApplication::UnicodeUTF8));
        menuHelp->setTitle(QApplication::translate("MainWindow", "Help", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H

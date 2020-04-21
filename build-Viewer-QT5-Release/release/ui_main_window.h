/********************************************************************************
** Form generated from reading UI file 'main_window.ui'
**
** Created by: Qt User Interface Compiler version 5.12.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAIN_WINDOW_H
#define UI_MAIN_WINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "glwidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionQuit;
    QAction *actionLoad;
    QAction *actionLoad_Specular;
    QAction *actionLoad_Diffuse;
    QWidget *Widget;
    QHBoxLayout *horizontalLayout;
    GLWidget *glwidget;
    QVBoxLayout *Configuration;
    QGroupBox *TreeOptions;
    QSpacerItem *Spacer;
    QGroupBox *RenderOptions;
    QLabel *Label_NumFaces;
    QLabel *Label_Faces;
    QLabel *Label_Vertices;
    QLabel *Label_NumVertices;
    QLabel *Label_Framerate;
    QLabel *Label_NumFramerate;
    QMenuBar *menuBar;
    QMenu *menuFile;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(828, 638);
        MainWindow->setMinimumSize(QSize(827, 618));
        MainWindow->setBaseSize(QSize(600, 600));
        actionQuit = new QAction(MainWindow);
        actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
        actionLoad = new QAction(MainWindow);
        actionLoad->setObjectName(QString::fromUtf8("actionLoad"));
        actionLoad_Specular = new QAction(MainWindow);
        actionLoad_Specular->setObjectName(QString::fromUtf8("actionLoad_Specular"));
        actionLoad_Diffuse = new QAction(MainWindow);
        actionLoad_Diffuse->setObjectName(QString::fromUtf8("actionLoad_Diffuse"));
        Widget = new QWidget(MainWindow);
        Widget->setObjectName(QString::fromUtf8("Widget"));
        Widget->setMinimumSize(QSize(827, 0));
        Widget->setBaseSize(QSize(600, 600));
        horizontalLayout = new QHBoxLayout(Widget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        glwidget = new GLWidget(Widget);
        glwidget->setObjectName(QString::fromUtf8("glwidget"));
        glwidget->setMinimumSize(QSize(600, 600));
        glwidget->setMaximumSize(QSize(16777215, 16777215));
        glwidget->setBaseSize(QSize(600, 600));

        horizontalLayout->addWidget(glwidget);

        Configuration = new QVBoxLayout();
        Configuration->setSpacing(6);
        Configuration->setObjectName(QString::fromUtf8("Configuration"));
        TreeOptions = new QGroupBox(Widget);
        TreeOptions->setObjectName(QString::fromUtf8("TreeOptions"));
        TreeOptions->setMinimumSize(QSize(200, 0));
        TreeOptions->setMaximumSize(QSize(200, 16777215));

        Configuration->addWidget(TreeOptions);

        Spacer = new QSpacerItem(50, 50, QSizePolicy::Minimum, QSizePolicy::Maximum);

        Configuration->addItem(Spacer);

        RenderOptions = new QGroupBox(Widget);
        RenderOptions->setObjectName(QString::fromUtf8("RenderOptions"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(RenderOptions->sizePolicy().hasHeightForWidth());
        RenderOptions->setSizePolicy(sizePolicy);
        RenderOptions->setMaximumSize(QSize(200, 120));
        RenderOptions->setBaseSize(QSize(0, 100));
        Label_NumFaces = new QLabel(RenderOptions);
        Label_NumFaces->setObjectName(QString::fromUtf8("Label_NumFaces"));
        Label_NumFaces->setGeometry(QRect(90, 40, 91, 17));
        Label_Faces = new QLabel(RenderOptions);
        Label_Faces->setObjectName(QString::fromUtf8("Label_Faces"));
        Label_Faces->setGeometry(QRect(10, 40, 67, 17));
        Label_Vertices = new QLabel(RenderOptions);
        Label_Vertices->setObjectName(QString::fromUtf8("Label_Vertices"));
        Label_Vertices->setGeometry(QRect(10, 60, 67, 17));
        Label_NumVertices = new QLabel(RenderOptions);
        Label_NumVertices->setObjectName(QString::fromUtf8("Label_NumVertices"));
        Label_NumVertices->setGeometry(QRect(90, 60, 91, 17));
        Label_Framerate = new QLabel(RenderOptions);
        Label_Framerate->setObjectName(QString::fromUtf8("Label_Framerate"));
        Label_Framerate->setGeometry(QRect(10, 90, 71, 17));
        Label_NumFramerate = new QLabel(RenderOptions);
        Label_NumFramerate->setObjectName(QString::fromUtf8("Label_NumFramerate"));
        Label_NumFramerate->setGeometry(QRect(90, 90, 91, 17));

        Configuration->addWidget(RenderOptions);


        horizontalLayout->addLayout(Configuration);

        MainWindow->setCentralWidget(Widget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 828, 22));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        MainWindow->setMenuBar(menuBar);

        menuBar->addAction(menuFile->menuAction());
        menuFile->addAction(actionQuit);
        menuFile->addAction(actionLoad);

        retranslateUi(MainWindow);
        QObject::connect(glwidget, SIGNAL(SetFaces(QString)), Label_NumFaces, SLOT(setText(QString)));
        QObject::connect(glwidget, SIGNAL(SetVertices(QString)), Label_NumVertices, SLOT(setText(QString)));
        QObject::connect(glwidget, SIGNAL(SetFramerate(QString)), Label_NumFramerate, SLOT(setText(QString)));

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        actionQuit->setText(QApplication::translate("MainWindow", "Quit", nullptr));
        actionLoad->setText(QApplication::translate("MainWindow", "Load", nullptr));
        actionLoad_Specular->setText(QApplication::translate("MainWindow", "Load Specular", nullptr));
        actionLoad_Diffuse->setText(QApplication::translate("MainWindow", "Load Diffuse", nullptr));
        TreeOptions->setTitle(QApplication::translate("MainWindow", "Options", nullptr));
        RenderOptions->setTitle(QApplication::translate("MainWindow", "Information", nullptr));
        Label_NumFaces->setText(QApplication::translate("MainWindow", "0", nullptr));
        Label_Faces->setText(QApplication::translate("MainWindow", "Faces", nullptr));
        Label_Vertices->setText(QApplication::translate("MainWindow", "Vertices", nullptr));
        Label_NumVertices->setText(QApplication::translate("MainWindow", "0", nullptr));
        Label_Framerate->setText(QApplication::translate("MainWindow", "Framerate", nullptr));
        Label_NumFramerate->setText(QApplication::translate("MainWindow", "0", nullptr));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAIN_WINDOW_H

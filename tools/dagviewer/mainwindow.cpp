
#include <QFileDialog>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), glWidget(new TestGLWidget(this)), editDialog(new EditDialog(this))
{
    ui->setupUi(this);

    setCentralWidget(glWidget);

    resize(800, 600);

    // Connect the view controls to the view widget
    connect(editDialog, SIGNAL(scaleXChanged(float)), glWidget, SLOT(setScaleX(float)));
    connect(editDialog, SIGNAL(scaleYChanged(float)), glWidget, SLOT(setScaleY(float)));
    connect(editDialog, SIGNAL(scaleZChanged(float)), glWidget, SLOT(setScaleZ(float)));
    connect(editDialog, SIGNAL(rotationChanged(float)), glWidget, SLOT(setRotation(float)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_Exit_triggered()
{
    close();
}

void MainWindow::on_action_Open_DAG_File_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open DAG File"), "", tr("Depth Art Geometry (*.dag)"));

    if (!fileName.isNull())
        glWidget->open(fileName);
}

void MainWindow::on_actionEdit_Controls_toggled(bool value)
{
    editDialog->setVisible(value);
}

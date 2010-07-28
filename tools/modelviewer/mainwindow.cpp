
#include <QTimer>
#include <QFileDialog>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "bonehierarchy.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), glWidget(new Viewer(this)), editDialog(new EditDialog(this)),
    boneHierarchy(new BoneHierarchy(this)), animations(new AnimationsDialog(this))
{
    ui->setupUi(this);

    setCentralWidget(glWidget);

    resize(800, 600);

    // Connect the view controls to the view widget
    connect(editDialog, SIGNAL(scaleChanged(float)), glWidget, SLOT(setScale(float)));
    connect(editDialog, SIGNAL(rotationChanged(float)), glWidget, SLOT(setRotation(float)));

    ui->actionShow_Geometry->setChecked(glWidget->isDrawGeometry());
    ui->actionShow_Vertex_Normals->setChecked(glWidget->showNormals());
    ui->actionShow_Ground_Plane->setChecked(glWidget->isRenderGroundPlane());

    connect(boneHierarchy, SIGNAL(selectBone(int)), glWidget, SLOT(selectBone(int)));
    connect(animations, SIGNAL(playAnimation(QString)), glWidget, SLOT(playAnimation(QString)));

    QTimer *timer = new QTimer(this);
    timer->setInterval(1000 / 30);
    timer->setSingleShot(false);
    connect(timer, SIGNAL(timeout()), glWidget, SLOT(nextFrame()));
    timer->start();
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
        tr("Open Model"), "", tr("Model (*.model)"));

    if (!fileName.isNull()) {
        boneHierarchy->setSkeleton(NULL);
        glWidget->open(fileName);
        boneHierarchy->setSkeleton(glWidget->skeleton());
        animations->setAnimations(glWidget->animations());
    }
}

void MainWindow::on_actionEdit_Controls_toggled(bool value)
{
    editDialog->setVisible(value);
}

void MainWindow::on_actionShow_Vertex_Normals_toggled(bool showVertices)
{
    glWidget->setShowNormals(showVertices);
}

void MainWindow::on_actionShow_Geometry_toggled(bool draw)
{
    glWidget->setDrawGeometry(draw);
}

void MainWindow::on_actionShow_Bones_toggled(bool draw)
{
    glWidget->setDrawBones(draw);
}

void MainWindow::on_actionShow_Cloth_Bones_toggled(bool enabled)
{
    glWidget->setDrawClothBones(enabled);
}

void MainWindow::on_actionShow_Bone_Names_toggled(bool enabled)
{
    glWidget->setShowBoneNames(enabled);
}

void MainWindow::on_actionShow_Ground_Plane_toggled(bool enabled)
{
    glWidget->setRenderGroundPlane(enabled);
}

void MainWindow::on_actionBone_Hierarchy_triggered()
{
    boneHierarchy->show();
}

void MainWindow::on_actionAnimations_triggered()
{
    animations->show();
}

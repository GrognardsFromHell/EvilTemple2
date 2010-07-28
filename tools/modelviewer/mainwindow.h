#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "viewer.h"
#include "editdialog.h"
#include "bonehierarchy.h"
#include "animationsdialog.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    Viewer *glWidget;

    EditDialog *editDialog;

    BoneHierarchy *boneHierarchy;

    AnimationsDialog *animations;

private slots:
    void on_actionAnimations_triggered();
    void on_actionBone_Hierarchy_triggered();
    void on_actionShow_Ground_Plane_toggled(bool );
    void on_actionShow_Bone_Names_toggled(bool );
    void on_actionShow_Cloth_Bones_toggled(bool );
    void on_actionShow_Bones_toggled(bool );
    void on_actionShow_Geometry_toggled(bool );
    void on_actionShow_Vertex_Normals_toggled(bool );
    void on_actionEdit_Controls_toggled(bool );
    void on_action_Open_DAG_File_triggered();
    void on_action_Exit_triggered();
};

#endif // MAINWINDOW_H

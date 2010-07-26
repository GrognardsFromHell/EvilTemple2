#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "testglwidget.h"
#include "editdialog.h"

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

    TestGLWidget *glWidget;

    EditDialog *editDialog;

private slots:
    void on_actionEdit_Controls_toggled(bool );
    void on_action_Open_DAG_File_triggered();
    void on_action_Exit_triggered();
};

#endif // MAINWINDOW_H

#ifndef VIEWERWINDOW_H
#define VIEWERWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>

namespace Ui {
    class ViewerWindow;
}

class ViewerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ViewerWindow(QWidget *parent = 0);
    ~ViewerWindow();

private:
    Ui::ViewerWindow *ui;

    QGraphicsScene scene;
private slots:
    void on_actionOpen_triggered();
    void on_actionExit_triggered();
};

#endif // VIEWERWINDOW_H

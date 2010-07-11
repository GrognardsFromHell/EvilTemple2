#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gameglobal.h"

#include <QMainWindow>
#include <QDeclarativeView>
#include <QVariant>

class QGraphicsView;
class QDeclarativeItem;

namespace EvilTemple {

class Game;
class MainWindowData;

class GAME_EXPORT MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const Game &game, QWidget *parent = 0);
    ~MainWindow();

public slots:
    void readSettings();
    void writeSettings();
    void showFromSettings();

    void updateTitle();
    void viewStatusChanged(QDeclarativeView::Status status);

protected:
    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *);

private:
    QScopedPointer<MainWindowData> d_ptr;
};

}

#endif // MAINWINDOW_H

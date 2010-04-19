#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gameglobal.h"

#include <QMainWindow>
#include <QDeclarativeView>
#include <QVariant>

class QGraphicsView;
class QDeclarativeItem;

namespace EvilTemple {

class GameGraphicsScene;
class GameGraphicsView;
class Game;

class GAME_EXPORT MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const Game &game, QWidget *parent = 0);
    ~MainWindow();

signals:
    void consoleToggled();
    void logMessage(QVariant message, QVariant type);

public slots:
    void readSettings();
    void writeSettings();
    void showFromSettings();

    void updateTitle();
    void viewStatusChanged(QDeclarativeView::Status status);
    void consoleMessage(QtMsgType type, const char *message);

protected:
    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *);

private:        
    const Game &game;
    GameGraphicsScene *scene;
    QDeclarativeView *view;
    QDeclarativeItem *consoleWidget;
};

}

#endif // MAINWINDOW_H

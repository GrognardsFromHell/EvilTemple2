#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QGraphicsView;

namespace EvilTemple {

class GameGraphicsScene;
class Game;

class MainWindow : public QMainWindow
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

protected:
    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *);

private:        
    const Game &game;
    GameGraphicsScene *scene;
    QGraphicsView *view;
};

}

#endif // MAINWINDOW_H

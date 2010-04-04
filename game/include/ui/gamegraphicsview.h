#ifndef GAMEGRAPHICSVIEW_H
#define GAMEGRAPHICSVIEW_H

#include <QGraphicsView>

class QResizeEvent;

namespace EvilTemple
{

    class Game;

    class GameGraphicsView : public QGraphicsView
    {
    Q_OBJECT
    public:
        explicit GameGraphicsView(const Game &game, QWidget *parent = 0);

    signals:

    public slots:

    protected:
        void resizeEvent(QResizeEvent *event);

        const Game &game;
    };

}

#endif // GAMEGRAPHICSVIEW_H

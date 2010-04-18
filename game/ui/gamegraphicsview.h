#ifndef GAMEGRAPHICSVIEW_H
#define GAMEGRAPHICSVIEW_H

#include "gameglobal.h"

#include <QDeclarativeView>

class QResizeEvent;

namespace EvilTemple
{

    class Game;

    class GAME_EXPORT GameGraphicsView : public QDeclarativeView
    {
    Q_OBJECT
    public:
        GameGraphicsView(const Game &game, QWidget *parent = 0);

    signals:

    public slots:

    protected:
        void resizeEvent(QResizeEvent *event);

        const Game &game;
    };

}

#endif // GAMEGRAPHICSVIEW_H

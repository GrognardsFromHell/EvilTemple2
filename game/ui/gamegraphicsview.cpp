#include "ui/gamegraphicsview.h"

#include <QMouseEvent>
#include <QVector2D>
#include "ui/gamegraphicsscene.h"
#include "game.h"
#include "camera.h"

namespace EvilTemple
{

    GameGraphicsView::GameGraphicsView(const Game &_game, QWidget *parent) :
        QDeclarativeView(parent),
        game(_game)
    {
    }

    void GameGraphicsView::resizeEvent(QResizeEvent *event)
    {
        const QSize &size = event->size();
        scene()->setSceneRect(0, 0, size.width(), size.height());
    }

}

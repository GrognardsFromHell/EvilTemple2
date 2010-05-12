#ifndef GAMEGRAPHICSSCENE_H
#define GAMEGRAPHICSSCENE_H

#include "gameglobal.h"

#include <QGraphicsScene>
#include <QImage>
#include <QTime>
#include <QGraphicsSceneMouseEvent>
#include <QVector2D>

namespace EvilTemple {

    class Game;
    class GeometryMeshObject;
    class ConsoleWidget;

    class GameGraphicsSceneData;

    class GAME_EXPORT GameGraphicsScene : public QGraphicsScene
    {
        Q_OBJECT
    public:
        GameGraphicsScene(const Game &game, QObject *parent);
        ~GameGraphicsScene();

        double fps() const;

    public slots:
        /**
         Gets the number of objects that were drawn during the last update.
         */
        int objectsDrawn() const;

    protected:
        void initializeGL();
        void resizeGL(int w, int h);
        void paintGL();

    private:
        QScopedPointer<GameGraphicsSceneData> d_ptr;
    };

}

#endif // GAMEGRAPHICSSCENE_H

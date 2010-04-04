#ifndef GAMEGRAPHICSSCENE_H
#define GAMEGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QImage>
#include <QTime>
#include <QGraphicsSceneMouseEvent>
#include <QVector2D>

namespace EvilTemple {

    class Game;
    class GeometryMeshObject;
    class ConsoleWidget;

    class GameGraphicsScene : public QGraphicsScene
    {
        Q_OBJECT
    public:
        GameGraphicsScene(const Game &game, QObject *parent);
        ~GameGraphicsScene();

        double fps() const;

    protected:
        void drawBackground(QPainter *painter, const QRectF &rect);

        void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    public slots:
        void rotate();
        void toggleConsole();

        /**
      Gets the number of objects that were drawn during the last update.
      */
        int objectsDrawn() const;

    protected slots:
        void resizeConsole(const QRectF &viewport);

    private:
        bool dragging;
        QTime timer;
        int totalTime;
        int totalFrames;

        const Game &game;
        QScopedPointer<GeometryMeshObject> model;
        float rotx, roty;
        int _objectsDrawn;
        QVector2D _lastMousePos;
        ConsoleWidget *console;

        GeometryMeshObject *pickObject(QVector2D screenPos);
    };

    inline int GameGraphicsScene::objectsDrawn() const
    {
        return _objectsDrawn;
    }

}

#endif // GAMEGRAPHICSSCENE_H

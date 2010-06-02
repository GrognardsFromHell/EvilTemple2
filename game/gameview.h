#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QGraphicsView>
#include <QtDeclarative/QDeclarativeEngine>

#include "clippinggeometry.h"
#include "modelfile.h"

namespace EvilTemple {

class GameViewData;
class BackgroundMap;
class Scene;
class ClippingGeometry;

class GameView : public QGraphicsView
{
    Q_OBJECT
    Q_PROPERTY(BackgroundMap *backgroundMap READ backgroundMap)
    Q_PROPERTY(Scene *scene READ scene)
    Q_PROPERTY(ClippingGeometry *clippingGeometry READ clippingGeometry)
public:
    explicit GameView(QWidget *parent = 0);
    ~GameView();

    QDeclarativeEngine *uiEngine();

    BackgroundMap *backgroundMap() const;
    Scene *scene() const;
    ClippingGeometry *clippingGeometry() const;

signals:

public slots:

    void showView(const QString &url);

    void centerOnWorld(float worldX, float worldY);

    QPoint screenCenter() const;

    int objectsDrawn() const;

    void objectMousePressed();    

    SharedModel loadModel(const QString &filename);

protected:
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void drawBackground(QPainter *painter, const QRectF &rect);

private:
    QScopedPointer<GameViewData> d;    

};

}

#endif // GAMEVIEW_H

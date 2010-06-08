#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QGraphicsView>
#include <QtDeclarative/QDeclarativeEngine>

#include "clippinggeometry.h"
#include "modelfile.h"
#include "translations.h"

namespace EvilTemple {

class GameViewData;
class BackgroundMap;
class Scene;
class ClippingGeometry;
class Materials;
class ParticleSystems;

class GameView : public QGraphicsView
{
    Q_OBJECT
    Q_PROPERTY(BackgroundMap *backgroundMap READ backgroundMap)
    Q_PROPERTY(Scene *scene READ scene)
    Q_PROPERTY(ClippingGeometry *clippingGeometry READ clippingGeometry)
    Q_PROPERTY(Materials *materials READ materials)
    Q_PROPERTY(ParticleSystems* particleSystems READ particleSystems)

    Q_PROPERTY(int scrollBoxMinX READ scrollBoxMinX WRITE setScrollBoxMinX)
    Q_PROPERTY(int scrollBoxMinY READ scrollBoxMinY WRITE setScrollBoxMinY)
    Q_PROPERTY(int scrollBoxMaxX READ scrollBoxMaxX WRITE setScrollBoxMaxX)
    Q_PROPERTY(int scrollBoxMaxY READ scrollBoxMaxY WRITE setScrollBoxMaxY)

    Q_PROPERTY(QSize viewportSize READ viewportSize NOTIFY viewportChanged)
public:
    explicit GameView(QWidget *parent = 0);
    ~GameView();

    QDeclarativeEngine *uiEngine();

    BackgroundMap *backgroundMap() const;
    Scene *scene() const;
    ClippingGeometry *clippingGeometry() const;
    Materials *materials() const;
    ParticleSystems *particleSystems() const;

    int scrollBoxMinX() const {
        return mScrollBoxMinX;
    }

    int scrollBoxMinY() const {
        return mScrollBoxMinY;
    }

    int scrollBoxMaxX() const {
        return mScrollBoxMaxX;
    }

    int scrollBoxMaxY() const {
        return mScrollBoxMaxY;
    }

    void setScrollBoxMinX(int value) {
        mScrollBoxMinX = value;
    }

    void setScrollBoxMinY(int value) {
        mScrollBoxMinY = value;
    }

    void setScrollBoxMaxX(int value) {
        mScrollBoxMaxX = value;
    }

    void setScrollBoxMaxY(int value) {
        mScrollBoxMaxY = value;
    }

    const QSize &viewportSize() const;

    Translations *translations() const;

signals:

    void viewportChanged();

public slots:

    QObject *showView(const QString &url);

    QObject *addGuiItem(const QString &url);

    void centerOnWorld(float worldX, float worldY);

    QPoint screenCenter() const;

    int objectsDrawn() const;

    SharedModel loadModel(const QString &filename);

protected:
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void drawBackground(QPainter *painter, const QRectF &rect);

private:
    int mScrollBoxMinX, mScrollBoxMaxX, mScrollBoxMinY, mScrollBoxMaxY;

    QScopedPointer<GameViewData> d;    

};

}

#endif // GAMEVIEW_H

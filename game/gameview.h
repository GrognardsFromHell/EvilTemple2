#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QGraphicsView>
#include <QtDeclarative/QDeclarativeEngine>

#include "clippinggeometry.h"
#include "modelfile.h"
#include "translations.h"

namespace EvilTemple {

class GameViewData;
class Scene;
class ClippingGeometry;
class Materials;
class ParticleSystems;
class AudioEngine;
class SectorMap;
class Models;

class GameView : public QGraphicsView
{
    Q_OBJECT
    Q_PROPERTY(EvilTemple::Scene *scene READ scene)
    Q_PROPERTY(ClippingGeometry *clippingGeometry READ clippingGeometry)
    Q_PROPERTY(Materials *materials READ materials)
    Q_PROPERTY(ParticleSystems* particleSystems READ particleSystems)
    Q_PROPERTY(AudioEngine* audioEngine READ audioEngine)
    Q_PROPERTY(SectorMap* sectorMap READ sectorMap)
    Q_PROPERTY(Models* models READ models)

    Q_PROPERTY(int scrollBoxMinX READ scrollBoxMinX WRITE setScrollBoxMinX)
    Q_PROPERTY(int scrollBoxMinY READ scrollBoxMinY WRITE setScrollBoxMinY)
    Q_PROPERTY(int scrollBoxMaxX READ scrollBoxMaxX WRITE setScrollBoxMaxX)
    Q_PROPERTY(int scrollBoxMaxY READ scrollBoxMaxY WRITE setScrollBoxMaxY)

    Q_PROPERTY(int scrollingBorder READ scrollingBorder WRITE setScrollingBorder)

    Q_PROPERTY(bool scrollingDisabled READ isScrollingDisabled WRITE setScrollingDisabled)

    Q_PROPERTY(QString currentCursor READ currentCursor WRITE setCurrentCursor)

    Q_PROPERTY(QSize viewportSize READ viewportSize NOTIFY viewportChanged)
public:
    explicit GameView(QWidget *parent = 0);
    ~GameView();

    QDeclarativeEngine *uiEngine();

    Scene *scene() const;
    ClippingGeometry *clippingGeometry() const;
    Materials *materials() const;
    ParticleSystems *particleSystems() const;
    AudioEngine *audioEngine() const;
    SectorMap *sectorMap() const;

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

    int scrollingBorder() {
        return mScrollingBorder;
    }

    void setScrollingBorder(int border) {
        mScrollingBorder = border;
    }

    const QSize &viewportSize() const;

    Translations *translations() const;

    Models *models() const;

    void setScrollingDisabled(bool disabled);
    bool isScrollingDisabled() const;

    const QString &currentCursor() const;
    void setCurrentCursor(const QString &filename);

signals:

    void viewportChanged();

    void worldClicked(QMouseEvent *event, const Vector4 &position);

    void worldDoubleClicked(QMouseEvent *event, const Vector4 &position);

public slots:

    QObject *showView(const QString &url);

    QObject *addGuiItem(const QString &url);

    void centerOnWorld(const Vector4 &position);

    Vector4 worldCenter() const;

    int objectsDrawn() const;

    void addVisualTimer(uint elapseAfter, const QScriptValue &callback);

    QUrl takeScreenshot();

    void deleteScreenshot(const QUrl &url);

    QString readBase64(const QUrl &file);

    void openBrowser(const QUrl &url);

    bool playMovie(const QString &filename, const QScriptValue &callback);

    bool playUiSound(const QString &filename);

    QImage getFrameTimeHistogram();

    float getFrameTimeAverage();

    /**
      Gets the screen coordinates (relative to the current viewport)
      from the given world coordinates.
      */
    Vector4 screenFromWorld(const Vector4 &position) const;

protected:
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);

    void drawBackground(QPainter *painter, const QRectF &rect);

private:
    int mScrollBoxMinX, mScrollBoxMaxX, mScrollBoxMinY, mScrollBoxMaxY;
    int mScrollingBorder;

    QScopedPointer<GameViewData> d;

};

}

#endif // GAMEVIEW_H

#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QGraphicsView>
#include <QtDeclarative/QDeclarativeEngine>

namespace EvilTemple {

class GameViewData;

class GameView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GameView(QWidget *parent = 0);
    ~GameView();

    QDeclarativeEngine *uiEngine();

signals:

public slots:

    void showView(const QString &url);

    void centerOnWorld(float worldX, float worldY);

    QPoint screenCenter() const;

    int objectsDrawn() const;

    void objectMousePressed();

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

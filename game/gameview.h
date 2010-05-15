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

    void showView(const QString &url);

signals:

public slots:

    void centerOnWorld(float worldX, float worldY);

    QPoint screenCenter() const;

protected:
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    QScopedPointer<GameViewData> d;
    void drawBackground(QPainter *painter, const QRectF &rect);

};

}

#endif // GAMEVIEW_H

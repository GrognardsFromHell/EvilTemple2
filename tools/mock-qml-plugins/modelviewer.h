#ifndef MODELVIEWER_H
#define MODELVIEWER_H

#include <QDeclarativeItem>
#include <QScopedPointer>

class ModelViewer : public QDeclarativeItem
{
Q_OBJECT
Q_PROPERTY(float modelScale READ modelScale WRITE setModelScale)
public:
    ModelViewer();

    void setModelScale(float modelScale);
    float modelScale() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
private:
    float mModelScale;
};

inline void ModelViewer::setModelScale(float modelScale)
{
    mModelScale = modelScale;
    update();
}

inline float ModelViewer::modelScale() const
{
    return mModelScale;
}

#endif // MODELVIEWER_H

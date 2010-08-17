
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>

#include "modelviewer.h"

ModelViewer::ModelViewer() : mModelScale(1)
{
    setFlag(ItemHasNoContents, false);
}

void ModelViewer::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->fillRect(QRectF(0, 0, width(), height()), QColor(255, 0, 0));
    painter->drawText(0, 20, QString("Scale: %1").arg(mModelScale));
}

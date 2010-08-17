#ifndef MODELVIEWER_H
#define MODELVIEWER_H

#include <GL/glew.h>

#include <QMetaType>
#include <QDeclarativeItem>
#include <QScopedPointer>

#include "materials.h"
#include "scene.h"
#include "renderstates.h"

namespace EvilTemple {

class ModelViewerData;

class ModelViewer : public QDeclarativeItem
{
    Q_OBJECT
    Q_PROPERTY(float modelScale READ modelScale WRITE setModelScale)
    Q_PROPERTY(float modelRotation READ modelRotation WRITE setModelRotation)
    Q_PROPERTY(ModelInstance *modelInstance READ modelInstance)
    Q_PROPERTY(Materials *materials READ materials)
public:
    ModelViewer();
    ~ModelViewer();

    float modelScale() const;
    void setModelScale(float value);

    float modelRotation() const;
    void setModelRotation(float rotation);

    ModelInstance *modelInstance() const;
    Materials *materials() const;

    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
protected:
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);

signals:

public slots:

private:
    float mModelScale;
    float mModelRotation;

    QScopedPointer<ModelViewerData> d;

    void updateBuffers();
};

inline float ModelViewer::modelScale() const
{
    return mModelScale;
}

inline void ModelViewer::setModelScale(float value)
{
    mModelScale = value;
    update();
}

inline float ModelViewer::modelRotation() const
{
    return mModelRotation;
}

inline void ModelViewer::setModelRotation(float value)
{
    mModelRotation = value;
    update();
}

}

Q_DECLARE_METATYPE(EvilTemple::ModelViewer*)

#endif // MODELVIEWER_H

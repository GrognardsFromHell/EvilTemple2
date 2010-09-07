#ifndef BACKGROUNDMAP_H
#define BACKGROUNDMAP_H

#include "renderable.h"

#include <QMetaType>
#include <QObject>
#include <QScopedPointer>

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

class RenderStates;
class BackgroundMapData;

class BackgroundMap : public Renderable
{
Q_OBJECT
Q_PROPERTY(QString directory READ directory WRITE setDirectory)
Q_PROPERTY(Vector4 color READ color WRITE setColor)
public:
    BackgroundMap();
    ~BackgroundMap();

    void render(RenderStates &renderStates, MaterialState *overrideMaterial = NULL);

    const Box3d &boundingBox();

    bool setDirectory(const QString &directory);
    const QString &directory();

    void setColor(const Vector4 &color);
    const Vector4 &color() const;

    IntersectionResult intersect(const Ray3d &ray) const;

private:
    QScopedPointer<BackgroundMapData> d;
    Q_DISABLE_COPY(BackgroundMap)
};

}

Q_DECLARE_METATYPE(EvilTemple::BackgroundMap*)

#endif // BACKGROUNDMAP_H

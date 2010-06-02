#ifndef CLIPPINGGEOMETRY_H
#define CLIPPINGGEOMETRY_H

#include <QtCore/QScopedPointer>
#include <QtCore/QString>
#include "scene.h"

namespace EvilTemple {

class ClippingGeometryData;
class RenderStates;

/**
  This class handles all the clipping geometry on the map.
  */
class ClippingGeometry : public QObject
{
Q_OBJECT
public:
    ClippingGeometry(RenderStates &renderStates);
    ~ClippingGeometry();

public slots:
    bool load(const QString &filename, Scene *scene);
    void unload();

private:
    QScopedPointer<ClippingGeometryData> d;
};

}

Q_DECLARE_METATYPE(EvilTemple::ClippingGeometry*)

#endif // CLIPPINGGEOMETRY_H

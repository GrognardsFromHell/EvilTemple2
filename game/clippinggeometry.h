#ifndef CLIPPINGGEOMETRY_H
#define CLIPPINGGEOMETRY_H

#include <QtCore/QScopedPointer>
#include <QtCore/QString>

namespace EvilTemple {

class ClippingGeometryData;
class RenderStates;

/**
  This class handles all the clipping geometry on the map.
  */
class ClippingGeometry
{
public:
    ClippingGeometry(RenderStates &renderStates);
    ~ClippingGeometry();

    bool load(const QString &filename);
    void unload();

    void draw();

private:
    QScopedPointer<ClippingGeometryData> d;
};

}

#endif // CLIPPINGGEOMETRY_H

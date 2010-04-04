#ifndef CLIPPINGGEOMETRYOBJECT_H
#define CLIPPINGGEOMETRYOBJECT_H

#include <QObject>

#include "geometrymeshobject.h"

namespace EvilTemple
{

    class ClippingGeometryObject : public GeometryMeshObject
    {
    Q_OBJECT
    public:
        explicit ClippingGeometryObject(QObject *parent = 0);
        ~ClippingGeometryObject();

    protected:
        void updateWorldMatrix();

    private:
        Q_DISABLE_COPY(ClippingGeometryObject);
    };

}

#endif // CLIPPINGGEOMETRYOBJECT_H

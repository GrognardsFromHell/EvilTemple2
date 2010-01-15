
#include "zonetemplate.h"
#include "zonebackgroundmap.h"
#include "geometrymeshobject.h"

namespace EvilTemple
{

    class ZoneTemplateData {
    public:
        quint32 id;
        ZoneBackgroundMap *dayBackground;
        ZoneBackgroundMap *nightBackground;
        QList<GeometryMeshObject*> staticGeometry;
        QList<GeometryMeshObject*> clippingGeometry;
    };

    ZoneTemplate::ZoneTemplate(quint32 id, QObject *parent) :
            QObject(parent), d_ptr(new ZoneTemplateData)
    {
        d_ptr->id = id;
        d_ptr->dayBackground = NULL;
        d_ptr->nightBackground = NULL;
    }

    ZoneTemplate::~ZoneTemplate()
    {
    }

    quint32 ZoneTemplate::id() const
    {
        return d_ptr->id;
    }

    ZoneBackgroundMap *ZoneTemplate::dayBackground() const
    {
        return d_ptr->dayBackground;
    }

    ZoneBackgroundMap *ZoneTemplate::nightBackground() const
    {
        return d_ptr->nightBackground;
    }

    const QList<GeometryMeshObject*> &ZoneTemplate::staticGeometry() const
    {
        return d_ptr->staticGeometry;
    }

    const QList<GeometryMeshObject*> &ZoneTemplate::clippingGeometry() const
    {
        return d_ptr->clippingGeometry;
    }

    void ZoneTemplate::setDayBackground(ZoneBackgroundMap* backgroundMap)
    {
        d_ptr->dayBackground = backgroundMap;
    }

    void ZoneTemplate::setNightBackground(ZoneBackgroundMap* backgroundMap)
    {
        d_ptr->nightBackground = backgroundMap;
    }

    void ZoneTemplate::addStaticGeometry(GeometryMeshObject *object)
    {
        object->setParent(this);
        d_ptr->staticGeometry.append(object);
    }

    void ZoneTemplate::addClippingGeometry(GeometryMeshObject *object)
    {
        object->setParent(this);
        d_ptr->clippingGeometry.append(object);
    }

}

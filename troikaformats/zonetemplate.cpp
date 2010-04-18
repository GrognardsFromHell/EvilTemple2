
#include "zonetemplate.h"
#include "zonebackgroundmap.h"
#include "qbox3d.h"

namespace Troika
{

    class ZoneTemplateData {
    public:
        quint32 id;
        ZoneBackgroundMap *dayBackground;
        ZoneBackgroundMap *nightBackground;
        QList<GeometryObject*> staticGeometry;
        QList<GeometryObject*> clippingGeometry;

        QVector3D startPosition;
        quint32 movie; // Movie to play when entering

        bool tutorialMap;
        bool menuMap;
        bool unfogged;
        bool outdoor;
        bool dayNightTransfer;
        bool bedrest;

        QBox3D scrollBox;
        QString directory;
        QString name;
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

    ZoneBackgroundMap*ZoneTemplate::dayBackground() const
    {
        return d_ptr->dayBackground;
    }

    ZoneBackgroundMap *ZoneTemplate::nightBackground() const
    {
        return d_ptr->nightBackground;
    }

    const QList<GeometryObject*> &ZoneTemplate::staticGeometry() const
    {
        return d_ptr->staticGeometry;
    }

    const QList<GeometryObject*> &ZoneTemplate::clippingGeometry() const
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

    void ZoneTemplate::addStaticGeometry(GeometryObject *object)
    {
        d_ptr->staticGeometry.append(object);
    }

    void ZoneTemplate::addClippingGeometry(GeometryObject *object)
    {
        d_ptr->clippingGeometry.append(object);
    }

    const QVector3D ZoneTemplate::startPosition() const
    {
        return d_ptr->startPosition;
    }

    quint32 ZoneTemplate::movie()
    {
        return d_ptr->movie;
    }

    bool ZoneTemplate::isTutorialMap() const
    {
        return d_ptr->tutorialMap;
    }

    bool ZoneTemplate::isMenuMap() const
    {
        return d_ptr->menuMap;
    }

    bool ZoneTemplate::isUnfogged() const
    {
        return d_ptr->unfogged;
    }

    bool ZoneTemplate::isOutdoor() const
    {
        return d_ptr->outdoor;
    }

    bool ZoneTemplate::hasDayNightTransfer() const
    {
        return d_ptr->dayNightTransfer;
    }

    bool ZoneTemplate::allowsBedrest() const
    {
        return d_ptr->bedrest;
    }

    void ZoneTemplate::setStartPosition(const QVector3D &startPosition)
    {
        d_ptr->startPosition = startPosition;
    }

    void ZoneTemplate::setMovie(quint32 movie)
    {
        d_ptr->movie = movie;
    }

    void ZoneTemplate::setTutorialMap(bool enabled)
    {
        d_ptr->tutorialMap = enabled;
    }

    void ZoneTemplate::setMenuMap(bool enabled)
    {
        d_ptr->menuMap = enabled;
    }

    void ZoneTemplate::setUnfogged(bool enabled)
    {
        d_ptr->unfogged = enabled;
    }

    void ZoneTemplate::setOutdoor(bool enabled)
    {
        d_ptr->outdoor = enabled;
    }

    void ZoneTemplate::setDayNightTransfer(bool enabled)
    {
        d_ptr->dayNightTransfer = enabled;
    }

    void ZoneTemplate::setBedrest(bool enabled)
    {
        d_ptr->bedrest = enabled;
    }

    const QBox3D &ZoneTemplate::scrollBox() const
    {
        return d_ptr->scrollBox;
    }

    void ZoneTemplate::setScrollBox(const QBox3D &box)
    {
        d_ptr->scrollBox = box;
    }

    const QString &ZoneTemplate::directory() const
    {
        return d_ptr->directory;
    }

    void ZoneTemplate::setDirectory(const QString &directory)
    {
        d_ptr->directory = directory;
    }

    const QString &ZoneTemplate::name() const
    {
        return d_ptr->name;
    }

    void ZoneTemplate::setName(const QString &name)
    {
        d_ptr->name = name;
    }

}

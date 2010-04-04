
#include "campaign/zone.h"
#include "zonetemplate.h"

namespace EvilTemple
{

    class ZoneData
    {
    public:
        QSharedPointer<ZoneTemplate> zoneTemplate;
    };

    Zone::Zone(const QSharedPointer<ZoneTemplate> &zoneTemplate, QObject *parent) :
            QObject(parent), d_ptr(new ZoneData)
    {
        d_ptr->zoneTemplate = zoneTemplate;
    }

    Zone::~Zone()
    {
    }

    const QSharedPointer<ZoneTemplate> &Zone::zoneTemplate() const
    {
        return d_ptr->zoneTemplate;
    }

}

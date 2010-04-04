
#include <QSharedPointer>
#include <QList>

#include "zonetemplate.h"
#include "campaign/campaign.h"
#include "campaign/zone.h"
#include "game.h"
#include "zonetemplates.h"

namespace EvilTemple
{

    class CampaignData
    {
    public:
        Zone* currentZone;
        QList<Zone*> visitedZones;
        const Game *game;
    };

    Campaign::Campaign(Game *game) :
        QObject(game),
        d_ptr(new CampaignData)
    {
        d_ptr->game = game;
        d_ptr->currentZone = NULL;
    }

    Campaign::~Campaign()
    {
    }

    Zone *Campaign::currentZone() const
    {
        return d_ptr->currentZone;
    }

    const QList<Zone*> &Campaign::visitedZones() const
    {
        return d_ptr->visitedZones;
    }

    void Campaign::loadMap(quint32 id)
    {
        if (d_ptr->currentZone && d_ptr->currentZone->zoneTemplate()->id() != id)
        {
            // TODO: Unload current map
        }

        foreach (Zone *zone, d_ptr->visitedZones)
        {
            if (zone->zoneTemplate()->id() == id)
            {
                d_ptr->currentZone = zone;
                // TODO: Event stuff
                return;
            }
        }

        const QSharedPointer<ZoneTemplate> &tpl = d_ptr->game->zoneTemplates()->get(id);

        // Create a new zone based on the template and make it the current zone.
        Zone *zone = new Zone(tpl, this);
        d_ptr->visitedZones.append(zone);
        d_ptr->currentZone = zone;
        // TODO: Event stuff
    }

}

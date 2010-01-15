#ifndef CAMPAIGN_H
#define CAMPAIGN_H

#include <QObject>

namespace EvilTemple
{

    class CampaignData;
    class Zone;
    class Game;

    /**
      This class represents a campaign, which is the running game world.

      Campaigns are the main root object that is persisted to a savegame.
      */
    class Campaign : public QObject
    {
    Q_OBJECT
    public:
        explicit Campaign(QObject *parent = 0);
        ~Campaign();

        /**
          Gets the current zone.
          */
        Zone *currentZone() const;

        /**
          Returns a list of zones that were visited during the campaign.
          */
        const QList<Zone> &visitedZones() const;

        /**
          Loads a map.
          */
        void loadMap(const Game &game, quint32 id);

    signals:

    public slots:

    private:
        QScopedPointer<CampaignData> d_ptr;

        Q_DISABLE_COPY(Campaign)
    };

}

#endif // CAMPAIGN_H

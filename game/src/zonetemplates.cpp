
#include <QWeakPointer>
#include <QHash>

#include "zonetemplates.h"
#include "zonetemplate.h"
#include "io/messagefile.h"
#include "io/virtualfilesystem.h"
#include "io/zonetemplatereader.h"
#include "game.h"

namespace EvilTemple
{

    const QString mapListFile("rules/MapList.mes");

    struct MapListEntry
    {
        /*
          Data from rules/MapList.mes
         */

        QString mapDirectory;
        quint32 startX;
        quint32 startY;
        quint32 movie;
        quint32 worldmap;

        bool tutorialMap;
        bool shoppingMap;
        bool startMap;
        bool unfogged;
        bool outdoor;
        bool dayNightTransfer;
        bool bedrest;

        /**
          Fill this map list entry with text from a MapList.mes value.
          @return True if the entry was parsed correctly.
          */
        bool parse(const QString &text)
        {
            QStringList parts = text.split(QRegExp("\\s*,\\s*"), QString::KeepEmptyParts);

            // Needs at least 3 parts (mapdir, x, y)
            if (parts.length() < 3)
                return false;

            bool startXOk, startYOk;

            mapDirectory = "maps/" + parts[0] + "/";
            startX = parts[1].toInt(&startXOk);
            startY = parts[2].toInt(&startYOk);

            if (!startXOk || !startYOk)
            {
                qWarning("Map entry has incorrect startx or starty: %s", qPrintable(text));
                return false;
            }

            QRegExp flagPattern("\\s*Flag:\\s*(\\w+)\\s*");
            QRegExp typePattern("\\s*Type:\\s*(\\w+)\\s*");
            QRegExp moviePattern("\\s*Movie:\\s*(\\d*)\\s*");
            QRegExp worldmapPattern("\\s*WorldMap:\\s*(\\d*)\\s*");

            for (int i = 3; i < parts.length(); ++i)
            {
                QString part = parts[i];
                if (flagPattern.exactMatch(part))
                {
                    QString flag = flagPattern.cap(1);
                    if (!flag.compare("DAYNIGHT_XFER", Qt::CaseInsensitive))
                    {
                        dayNightTransfer = true;
                    }
                    else if (!flag.compare("OUTDOOR", Qt::CaseInsensitive))
                    {
                        outdoor = true;
                    }
                    else if (!flag.compare("BEDREST", Qt::CaseInsensitive))
                    {
                        bedrest = true;
                    }
                    else if (!flag.compare("UNFOGGED", Qt::CaseInsensitive))
                    {
                        unfogged = true;
                    }
                    else
                    {
                        qWarning("Entry has unknown flag %s: %s", qPrintable(flag), qPrintable(text));
                        return false;
                    }
                }
                else if (typePattern.exactMatch(part))
                {
                    QString type = typePattern.cap(1);

                    if (!type.compare("SHOPPING_MAP", Qt::CaseInsensitive))
                    {
                        shoppingMap = true;
                    }
                    else if (!type.compare("START_MAP", Qt::CaseInsensitive))
                    {
                        startMap = true;
                    }
                    else if (!type.compare("TUTORIAL_MAP", Qt::CaseInsensitive))
                    {
                        tutorialMap = true;
                    }
                    else
                    {
                        qWarning("Entry has unknown type: %s: %s", qPrintable(type), qPrintable(text));
                        return false;
                    }
                }
                else if (moviePattern.exactMatch(part))
                {
                    movie = moviePattern.cap(1).toInt();
                }
                else if (worldmapPattern.exactMatch(part))
                {
                    worldmap = worldmapPattern.cap(1).toInt();
                }
                else
                {
                    qWarning("Unknown part in map list entry: %s", qPrintable(text));
                    return false;
                }
            }

            return true;
        }
    };

    /**
        Used for implementation hiding by ZoneTemplates.
      */
    class ZoneTemplatesData
    {
    public:
        // The type for the template cache
        typedef QHash< quint32, QWeakPointer<ZoneTemplate> > ZoneTemplateCache;

        const Game &game;
        VirtualFileSystem *vfs;
        ZoneTemplateCache cache;
        QMap<quint32, MapListEntry> mapList;

        ZoneTemplatesData(const Game &_game) : game(_game), vfs(game.virtualFileSystem()) {
            loadAvailableMaps();
        }

        /**
          Loads the list of available zones from the disk
          */
        void loadAvailableMaps()
        {
            QByteArray mapListData = vfs->openFile(mapListFile);

            if (mapListData.isNull())
            {
                qWarning("No map list file found: %s", qPrintable(mapListFile));
            }
            else
            {
                QHash<quint32,QString> entries = MessageFile::parse(mapListData);
                MapListEntry entry;

                QHash<quint32,QString>::iterator it;
                for (it = entries.begin(); it != entries.end(); ++it)
                {
                    if (entry.parse(it.value()))
                    {
                        mapList[it.key()] = entry;
                    }
                }
            }
        }

        /**
          Loads a zone template from disk. No caching is performed by this method.
          */
        QSharedPointer<ZoneTemplate> loadZoneTemplate(quint32 id)
        {
            QSharedPointer<ZoneTemplate> result(new ZoneTemplate(id));

            ZoneTemplateReader reader(game, result.data(), mapList[id].mapDirectory);
            reader.read();

            return result;
        }
    };

    ZoneTemplates::ZoneTemplates(const Game &game, QObject *parent) :
        QObject(parent),
        d_ptr(new ZoneTemplatesData(game))
    {        
    }

    ZoneTemplates::~ZoneTemplates()
    {

    }

    /**
      Eithers gets a zone template from the cache if it's still referenced somewhere or
      create a new zone template and put it into the cache.

      @param id The id of the map to load. This comes from MapList.mes.
      */
    QSharedPointer<ZoneTemplate> ZoneTemplates::get(quint32 id)
    {
        ZoneTemplatesData::ZoneTemplateCache::iterator it = d_ptr->cache.find(id);

        // Try to find an entry in the cache that is still valid
        if (it != d_ptr->cache.end())
        {
            QSharedPointer<ZoneTemplate> cachedEntry(*it);

            // See if the referenced object was stil valid
            if (cachedEntry)
            {
                return cachedEntry;
            }
            else
            {
                // The referenced object was deleted already > clean up cache.
                d_ptr->cache.erase(it);
            }
        }

        // Load the template from disk, store it in the cache and return the pointer.
        QSharedPointer<ZoneTemplate> result = d_ptr->loadZoneTemplate(id);

        d_ptr->cache.insert(id, QWeakPointer<ZoneTemplate>(result));

        return result;
    }

}

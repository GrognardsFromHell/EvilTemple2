
#include <QWeakPointer>
#include <QHash>
#include "qbox3d.h"

#include "zonetemplates.h"
#include "zonetemplate.h"
#include "messagefile.h"
#include "prototypes.h"
#include "virtualfilesystem.h"
#include "zonetemplatereader.h"
#include "constants.h"

namespace Troika
{

    const QString mapListFile("rules/MapList.mes");

    const QString mapLimitsFile("rules/MapLimits.mes");

    const QString mapNamesFile("mes/map_names.mes");

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
            tutorialMap = false;
            shoppingMap = false;
            startMap = false;
            unfogged = false;
            outdoor = false;
            dayNightTransfer = false;
            bedrest = false;
            movie = 0;
            worldmap = 0;
            startX = 0;
            startY = 0;

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

        ZoneTemplatesData(VirtualFileSystem *_vfs, Prototypes *_prototypes) : vfs(_vfs), prototypes(_prototypes) {
            loadAvailableMaps();
            loadMapLimits();
            mapNames = MessageFile::parse(vfs->openFile(mapNamesFile));
        }

        VirtualFileSystem *vfs;
        Prototypes *prototypes;
        QHash<quint32,MapListEntry> mapListEntries;
        QHash<quint32,QBox3D> mapLimits;
        QHash<quint32,QString> mapNames;

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
                        mapListEntries[it.key()] = entry;
                    }
                }
            }
        }

        void loadMapLimits() {
            QHash<quint32,QString> rawMapLimits = MessageFile::parse(vfs->openFile(mapLimitsFile));

            foreach(quint32 mapId, mapListEntries.keys()) {

                if (!rawMapLimits.contains(mapId)) {
                    qWarning("Map with id %d has no corresponding map limits entry.", mapId);
                    continue;
                } else {
                    QStringList parts = rawMapLimits[mapId].split(',');
                    int maxX = parts[0].toInt();
                    int maxY = parts[1].toInt();
                    int minX = parts[2].toInt();
                    int minY = parts[3].toInt();

                    QVector3D minBox(minX, minY, 0);
                    QVector3D maxBox(maxX, maxY, 0);

                    QBox3D scrollBox(minBox, maxBox);

                    mapLimits[mapId] = scrollBox;
                }
            }
        }

        ZoneTemplate *load(quint32 id) const {
            const MapListEntry &mapListEntry = mapListEntries[id];

            ZoneTemplate *result = new ZoneTemplate(id);
            // Transform to world coordinate
            float y = (mapListEntry.startY + 0.5f) * PixelPerWorldTile;
            float x = (mapListEntry.startX + 0.5f) * PixelPerWorldTile;
            result->setStartPosition(QPoint(x, y));
            result->setMovie(mapListEntry.movie);
            result->setBedrest(mapListEntry.bedrest);
            result->setMenuMap(mapListEntry.shoppingMap);
            result->setDayNightTransfer(mapListEntry.dayNightTransfer);
            result->setTutorialMap(mapListEntry.tutorialMap);
            result->setOutdoor(mapListEntry.outdoor);
            result->setUnfogged(mapListEntry.unfogged);
            result->setDirectory(mapListEntry.mapDirectory);
            result->setName(mapNames[id]);
            result->setScrollBox(mapLimits[id]);

            ZoneTemplateReader reader(vfs, prototypes, result, mapListEntry.mapDirectory);
            reader.read();

            return result;
        }
    };

    ZoneTemplates::ZoneTemplates(VirtualFileSystem *vfs, Prototypes *prototypes) :
        d_ptr(new ZoneTemplatesData(vfs, prototypes))
    {
    }

    ZoneTemplates::~ZoneTemplates()
    {

    }

    ZoneTemplate *ZoneTemplates::load(quint32 id)
    {
        return d_ptr->load(id);
    }

    QList<quint32> ZoneTemplates::mapIds() {
        return d_ptr->mapListEntries.keys();
    }

}

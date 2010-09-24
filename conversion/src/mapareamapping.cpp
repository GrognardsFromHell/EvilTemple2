
#include <QFile>
#include <QStringList>
#include <QHash>

#include "conversion/mapareamapping.h"

static bool initialized = false;

static QHash<uint, QString> mapping;

static void initialize()
{
    if (initialized)
        return;

    initialized = true;

    QFile file(":/map_area_mapping.txt");

    if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        qWarning("Unable to open the map area mapping.");
        return;
    }

    QString currentAreaSection;

    while (!file.atEnd()) {
        QString line = QString::fromLatin1(file.readLine()).trimmed();

        if (line.startsWith("#") || line.isEmpty())
            continue; // Comments

        if (line.startsWith("[") && line.endsWith("]")) {
            currentAreaSection = line.mid(1, line.length() - 2);
            continue;
        }

        if (line.contains("-")) {
            QStringList parts = line.split("-");
            bool okMin, okMax;
            uint from = parts[0].toUInt(&okMin);
            uint to = parts[1].toUInt(&okMax);

            if (!okMin || !okMax) {
                qWarning("Invalid line in map-id to area mapping: %s", qPrintable(line));
            } else {
                for (uint i = from; i <= to; ++i) {
                    mapping[i] = currentAreaSection;
                }
            }
        } else {
            bool ok;

            uint mapId = line.toUInt(&ok);

            if (!ok) {
                qWarning("Invalid line in map-id to area mapping: %s", qPrintable(line));
            } else {
                mapping[mapId] = currentAreaSection;
            }
        }
    }
}

QString getAreaFromMapId(uint mapId)
{
    initialize();

    return mapping[mapId];
}

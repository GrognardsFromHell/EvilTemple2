
#include <gamemath.h>
using namespace GameMath;

#include <QQuaternion>

#include "dagreader.h"
#include "messagefile.h"
#include "objectfilereader.h"
#include "skmreader.h"
#include "virtualfilesystem.h"
#include "zonetemplatereader.h"
#include "zonetemplate.h"
#include "zonebackgroundmap.h"
#include "troika_model.h"
#include "util.h"

namespace Troika
{

    const QString groundListFile("art/ground/ground.mes");

    ZoneTemplateReader::ZoneTemplateReader(VirtualFileSystem *_vfs,
                                           Prototypes *_prototypes,
                                           ZoneTemplate *_zoneTemplate,
                                           const QString &_mapDirectory) :
    vfs(_vfs),
    prototypes(_prototypes),
    zoneTemplate(_zoneTemplate),
    mapDirectory(_mapDirectory)
    {

        meshMapping = MessageFile::parse(vfs->openFile("art/meshes/meshes.mes"));

        foreach (uint key, meshMapping.keys()) {
            meshMapping[key] = "art/meshes/" + meshMapping[key];
        }

    }

    bool ZoneTemplateReader::read()
    {
        return readMapProperties()
                && readGroundDirectories()
                && readGeometryMeshFiles()
                && readGeometryMeshInstances()
                && readSectors()
                && readMobiles()
                && readClippingMeshFiles()
                && readClippingMeshInstances()
                && readGlobalLight()
                && readSoundSchemes();
    }

    bool ZoneTemplateReader::readMapProperties()
    {
        QByteArray mapProperties = vfs->openFile(mapDirectory + "map.prp");

        if (mapProperties.isNull())
            return false;

        QDataStream stream(mapProperties);
        stream.setByteOrder(QDataStream::LittleEndian);

        stream >> artId;

        // After this comes: unused 32-bit, width 64-bit and height 64-bit, which are always the same
        // for the stock maps.

        // Only ids 0-999 are usable, since id+1000 is the night-time map
        if (artId >= 1000)
        {
            qWarning("Map %s has invalid art id %d.", qPrintable(mapDirectory), artId);
            return false;
        }

        return true;
    }

    bool ZoneTemplateReader::readGroundDirectories()
    {
        QByteArray groundListData = vfs->openFile(groundListFile);

        if (groundListData.isNull())
        {
            qWarning("No ground list file found: %s", qPrintable(groundListFile));
            return false;
        }
        else
        {
            QHash<quint32,QString> entries = MessageFile::parse(groundListData);

            QString day = entries[artId];
            QString night = entries[1000 + artId];

            if (day.isEmpty())
            {
                qWarning("Map %s has no daylight background map.", qPrintable(mapDirectory));
                return false;
            }

            day.prepend("art/ground/");
            day.append("/");
            zoneTemplate->setDayBackground(new ZoneBackgroundMap(day, zoneTemplate));

            if (!night.isEmpty()) {
                night.prepend("art/ground/");
                night.append("/");
                zoneTemplate->setNightBackground(new ZoneBackgroundMap(night, zoneTemplate));
            }

        }

        return true;
    }

    bool ZoneTemplateReader::readGeometryMeshFiles()
    {
        QByteArray data = vfs->openFile(mapDirectory + "gmesh.gmf");

        if (data.isNull())
            return true; // Empty gmesh.gmf file

        QDataStream stream(data);
        stream.setByteOrder(QDataStream::LittleEndian);

        quint32 fileCount;
        stream >> fileCount;

        geometryMeshFiles.resize(fileCount);

        char filename[261];
        filename[260] = 0;

        for (quint32 i = 0; i < fileCount; ++i)
        {
            GeometryMeshFile &file = geometryMeshFiles[i];

            stream.readRawData(filename, 260);
            file.animationFilename = QString::fromLatin1(filename);

            stream.readRawData(filename, 260);
            file.modelFilename = QString::fromLatin1(filename);
        }

        return true;
    }

    bool ZoneTemplateReader::readGeometryMeshInstances()
    {
        QByteArray data = vfs->openFile(mapDirectory + "gmesh.gmi");

        if (data.isNull())
            return true; // No instances

        QDataStream stream(data);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        quint32 instanceCount;
        stream >> instanceCount;

        for (quint32 i = 0; i < instanceCount; ++i)
        {
            int fileIndex;
            float x, y, z, scaleX, scaleY, scaleZ, rotation;

            stream >> fileIndex >> x >> y >> z >> scaleX >> scaleY >> scaleZ >> rotation;

            // Check file index for sanity
            if (fileIndex < 0 || fileIndex >= geometryMeshFiles.size())
            {
                qWarning("Geometry mesh instance %d reference non-existant file %i in map %s.",
                         i, fileIndex, qPrintable(mapDirectory));
                continue;
            }

            // Convert radians to degrees
            rotation = rad2deg(rotation + LegacyBaseRotation);

            // Create the geometry mesh object and add it to the zone template.
            GeometryObject *meshObject = new GeometryObject(QVector3D(x, y, z),
                                                            rotation,
                                                            geometryMeshFiles[fileIndex].modelFilename);
            zoneTemplate->addStaticGeometry(meshObject);
        }

        return true;
    }

    bool ZoneTemplateReader::readSectors()
    {
        QStringList sectorFiles = vfs->listFiles(mapDirectory, "*.sec");

        foreach (QString sector, sectorFiles)
            readSector(sector);

        return true;
    }

    bool ZoneTemplateReader::readMobiles()
    {
        QStringList mobFiles = vfs->listFiles(mapDirectory, "*.mob");

        QHash<QString, GameObject*> gameObjects;
        QHash<QString, QString> filenameMapping;

        foreach (QString mobFile, mobFiles)
        {
            GameObject *gameObject = readMobile(mobFile);

            if (!gameObject)
                continue;

            Q_ASSERT(!gameObject->id.isNull()); // All mobiles need a GUID

            gameObjects[gameObject->id] = gameObject;
            filenameMapping[gameObject->id] = mobFile;

            if (gameObject->parentItemId.isNull()) {
                zoneTemplate->addMobile(gameObject);
            }
        }

        foreach (GameObject *gameObject, gameObjects) {
            if (gameObject->parentItemId.isNull())
                continue; // We already processed non-parented mobiles

            if (!gameObjects.contains(gameObject->parentItemId)) {
                qWarning("Skipping game object %s, since its parent %s is missing.",
                         qPrintable(filenameMapping[gameObject->id]),
                         qPrintable(gameObject->parentItemId));
                delete gameObject; // Free the object now that it won't be added to anything.
                continue;
            }

            gameObjects[gameObject->parentItemId]->content.append(gameObject);
        }

        return true;
    }

    GameObject *ZoneTemplateReader::readMobile(const QString &filename)
    {
        QByteArray data = vfs->openFile(filename);
        QDataStream stream(data);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        ObjectFileReader reader(prototypes, stream);
        reader.setFilename(filename);

        if (!reader.read(false)) {
            qWarning("%s (%s)", qPrintable(reader.errorMessage()), qPrintable(filename));
            return NULL;
        }

        return new GameObject(reader.getObject());
    }

    static void readSectorVisibility(TileSector &sector, const QString &filename, VirtualFileSystem *vfs)
    {
        QByteArray svbData = vfs->openFile(filename);

        if (svbData.isNull()) {
            for (int ty = 0; ty < SectorSidelength; ++ty)
                for (int tx = 0; tx < SectorSidelength; ++tx)
                    memset(sector.tiles[tx][ty].visibility, 0, sizeof(sector.tiles[tx][ty].visibility));
            return;
        }

        int i = 0;
        int j = 0;
        uchar b;

        for (int y = 0; y < SectorSidelength * 3; ++y)
        {
            for (int x = 0; x < SectorSidelength * 3; ++x)
            {
                uchar mask;

                if (j == 0) {
                    b = svbData[i++];
                    j = 1;
                    mask = b & 0xFF;
                } else if (j == 1) {
                    mask = (b >> 4) & 0xFF;
                    j = 0;
                }

                int tx = x / 3;
                int ty = y / 3;
                int sx = x % 3;
                int sy = y % 3;

                sector.tiles[tx][ty].visibility[sx][sy] = mask;
            }
        }
    }

    bool ZoneTemplateReader::readSector(const QString &filename)
    {
        QByteArray data = vfs->openFile(filename);
        QDataStream stream(data);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        QRegExp regexp("(\\d+)\\.sec");
        regexp.setCaseSensitivity(Qt::CaseInsensitive);

        if (regexp.indexIn(filename) == -1) {
            qWarning("Sector filename has invalid format: %s", qPrintable(filename));
            return false;
        }

        bool ok;
        uint sectorId = regexp.cap(1).toInt(&ok);

        if (!ok) {
            qWarning("Sector filename has invalid format: %s", qPrintable(filename));
            return false;
        }

        uint sectorY = (sectorId >> 26) & 0x3F;
        uint sectorX = sectorId & 0xFF;

        TileSector sector;
        sector.x = sectorX;
        sector.y = sectorY;
        sector.hasNegativeHeight = false;
        memset(sector.negativeHeight, 0, sizeof(sector.negativeHeight));

        if (!readSectorLights(stream) || !readSectorTiles(&sector, stream) || !readSectorObjects(stream))
            return false;

        // Try reading a SVB file for the sector.
        QString svbFilename = filename;
        svbFilename.replace(".sec", ".svb");

        readSectorVisibility(sector, svbFilename, vfs);

        // Try reading a HSD file for the sector.
        QString hsdFilename = zoneTemplate->directory() + "hsd" + QString::number(sectorId) + ".hsd";

        QByteArray hsdData = vfs->openFile(hsdFilename);

        if (!hsdData.isNull()) {
            QDataStream stream(hsdData);
            stream.setByteOrder(QDataStream::LittleEndian);

            uint version;
            stream >> version;

            if (version != 2) {
                qWarning("HSD file %s has version tag other than 2.", qPrintable(hsdFilename));
            } else {
                sector.hasNegativeHeight = false;

                for (int y = 0; y < SectorSidelength; ++y) {
                    for (int x = 0; x < SectorSidelength; ++x) {
                        for (int ty = 0; ty < 3; ++ty) {
                            for (int tx = 0; tx < 3; ++tx) {
                                stream >> sector.negativeHeight[x*3 + tx][y*3 + ty];

                                // For several sectors, there are *only* zeros
                                if (sector.negativeHeight[x+3 + tx][y*3 + ty] != 0) {
                                    sector.hasNegativeHeight = true;
                                }
                            }
                        }
                    }
                }
            }
        }

        zoneTemplate->addTileSector(sector);

        return true;
    }

    bool ZoneTemplateReader::readSectorLights(QDataStream &stream)
    {
        quint32 lightCount;
        stream >> lightCount;

        for (quint32 i = 0; i < lightCount; ++i)
        {
            quint32 flags;

            Light light;
            light.day = true;
            ParticleSystem particleSystem;

            quint32 xPos, yPos;
            float xOffset, yOffset, zOffset;

            stream >> light.handle >> flags >> light.type >> light.r >> light.b >> light.g >>
                    light.unknown >> light.ur >> light.ub >> light.ug >> light.ua >>
                    xPos >> yPos >> xOffset >> yOffset >> zOffset >>
                    light.dirX >> light.dirY >> light.dirZ >>
                    light.range >> light.phi;

            light.position = Vector4((xPos + .5f) * PixelPerWorldTile + xOffset,
                                     zOffset,
                                     (yPos + .5f) * PixelPerWorldTile + yOffset,
                                     1);

            // TODO: What to do about the direction?

            // Also check flags?
            if (light.type)
                zoneTemplate->addLight(light);

            if ((flags & 0x10) != 0 || (flags & 0x40) != 0)
            {
                particleSystem.light = light;
                stream >> particleSystem.hash >> particleSystem.id;
                if (particleSystem.hash)
                    zoneTemplate->addParticleSystem(particleSystem);
            }

            if ((flags & 0x40) != 0)
            {
                light.day = false;
                stream >> light.type >> light.r >> light.b >> light.g >>
                        light.unknown >>
                        light.dirX >> light.dirY >> light.dirZ >>
                        light.range >> light.phi;

                // Also check flags?
                if (light.type)
                    zoneTemplate->addLight(light);

                particleSystem.light = light;
                stream >> particleSystem.hash >> particleSystem.id;
                if (particleSystem.hash)
                    zoneTemplate->addParticleSystem(particleSystem);
            }
        }

        return true;
    }

    bool ZoneTemplateReader::readSectorTiles(TileSector *sector, QDataStream &stream)
    {
        for (int y = 0; y < SectorSidelength; ++y) {
            for (int x = 0; x < SectorSidelength; ++x) {
                SectorTile &tile = sector->tiles[x][y];
                stream >> tile.footstepsSound >> tile.unknown1[0] >> tile.unknown1[1] >> tile.unknown1[2]
                        >> tile.bitfield >> tile.unknown2;
            }
        }

        return stream.status() == QDataStream::Ok;
    }

    bool ZoneTemplateReader::readSectorObjects(QDataStream &stream)
    {
        int header;

        stream.skipRawData(48); // Skip unknown data

        // Read objects while the object header is valid
        stream >> header;

        int objectId = 0;

        while (header == ObjectFileVersion)
        {
            ObjectFileReader reader(prototypes, stream);
            reader.setFilename(QString("%1:%2").arg(mapDirectory).arg(objectId));

            if (!reader.read(true))
            {
                qWarning("Object file error: %s", qPrintable(reader.errorMessage()));
                return false;
            }

            Q_ASSERT(reader.getObject().parentItemId.isNull()); // No parent-child relation for static objects

            zoneTemplate->addStaticObject(new GameObject(reader.getObject()));

            stream >> header;

            if (header != ObjectFileVersion && !stream.atEnd())
            {
                int remaining = stream.device()->size() - stream.device()->pos();
                qWarning("Sector file has %d bytes beyond last object.", remaining);
            }
        }

        if (!stream.atEnd())
        {
            int remaining = stream.device()->size() - stream.device()->pos();
            qWarning("Sector file has %d bytes beyond last object.", remaining);
        }

        return true;
    }

    bool ZoneTemplateReader::readClippingMeshFiles()
    {
        QByteArray data = vfs->openFile(mapDirectory + "clipping.cgf");

        if (data.isNull())
            return true; // No instances

        QDataStream stream(data);
        stream.setByteOrder(QDataStream::LittleEndian);

        quint32 fileCount;
        stream >> fileCount;

        char filename[261];
        filename[260] = 0; // Ensure null termination

        for (quint32 i = 0; i < fileCount; ++i)
        {
            stream.readRawData(filename, 260);
            clippingMeshFiles.append(QString::fromLatin1(filename));
        }

        return true;
    }

    bool ZoneTemplateReader::readClippingMeshInstances()
    {
        using namespace GameMath;

        QByteArray data = vfs->openFile(mapDirectory + "clipping.cif");

        if (data.isNull())
            return true; // No instances

        QDataStream stream(data);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        quint32 instanceCount;
        stream >> instanceCount;

        for (quint32 i = 0; i < instanceCount; ++i) {
            qint32 meshIndex;
            stream >> meshIndex;

            if (meshIndex < 0 || meshIndex >= clippingMeshFiles.size()) {
                qWarning("Clipping geometry instance %i references invalid mesh: %d", i, meshIndex);
                return false;
            }

            QVector3D position, scale;
            stream >> position >> scale;

            float rotation;
            stream >> rotation;

            // TODO: The following transformation is incorrect and needs to be copied from the clipping geometry class

            GeometryObject *geometryMesh = new GeometryObject(position,
                                                              rotation,
                                                              scale,
                                                              clippingMeshFiles[meshIndex]);

            zoneTemplate->addClippingGeometry(geometryMesh);
        }

        return true;
    }

    static void readDaylightMes(const QHash<uint, QString> &daylightEntries,
                                uint baseId,
                                QList<LightKeyframe> &keyframesDay,
                                QList<LightKeyframe> &keyframesNight)
    {

        for (int i = 0; i < 24; ++i) {
            uint entryId = baseId + i;

            if (!daylightEntries.contains(entryId))
                continue;
;
            QVector<uint> numbers;
            bool entryInvalid = false;
            foreach (const QString &part, daylightEntries[entryId].split(',')) {
                bool ok;
                numbers.append(part.toUInt(&ok));
                if (!ok) {
                    entryInvalid = true;
                }
            }

            LightKeyframe keyframe;
            keyframe.hour = i;
            if (i != 6 && i != 18) {
                if (numbers.size() != 3) {
                    entryInvalid = true;
                } else {
                    keyframe.red = numbers[0] / 255.0f;
                    keyframe.green = numbers[1] / 255.0f;
                    keyframe.blue = numbers[2] / 255.0f;

                    if (i < 6 || i > 18)
                        keyframesNight.append(keyframe);
                    else
                        keyframesDay.append(keyframe);
                }
            } else if (i == 6) {
                keyframe.red = numbers[0] / 255.0f;
                keyframe.green = numbers[1] / 255.0f;
                keyframe.blue = numbers[2] / 255.0f;
                keyframesNight.append(keyframe);

                keyframe.red = numbers[3] / 255.0f;
                keyframe.green = numbers[4] / 255.0f;
                keyframe.blue = numbers[5] / 255.0f;
                keyframesDay.append(keyframe);
            } else if (i == 18) {
                keyframe.red = numbers[0] / 255.0f;
                keyframe.green = numbers[1] / 255.0f;
                keyframe.blue = numbers[2] / 255.0f;
                keyframesDay.append(keyframe);

                keyframe.red = numbers[3] / 255.0f;
                keyframe.green = numbers[4] / 255.0f;
                keyframe.blue = numbers[5] / 255.0f;
                keyframesNight.append(keyframe);
            }

            if (entryInvalid) {
                qWarning("Daylight.mes contains invalid entry %d.", entryId);
            }
        }
    }

    bool ZoneTemplateReader::readGlobalLight()
    {
        QByteArray data = vfs->openFile(mapDirectory + "global.lit");

        if (data.isNull()) {
            qWarning("Missing global lighting information for %s.", qPrintable(mapDirectory));
            return false;
        }

        QDataStream stream(data);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        Light globalLight;
        uint flags;
        float cr, cg, cb;
        stream >> flags >> globalLight.type >> cr >> cg >> cb;
        globalLight.r = cr * 255.0f;
        globalLight.g = cg * 255.0f;
        globalLight.b = cb * 255.0f;
        stream.skipRawData(3 * sizeof(float)); // Unknown data.
        stream >> globalLight.dirX >> globalLight.dirY >> globalLight.dirZ >> globalLight.range;

        // ToEE normalizes the global.lit direction after loading the file. We do it ahead of time here.
        Vector4 lightDir(globalLight.dirX, globalLight.dirY, globalLight.dirZ, 0);
        lightDir.normalize();
        globalLight.dirX = lightDir.x();
        globalLight.dirY = lightDir.y();
        globalLight.dirZ = lightDir.z();

        // NOTE: global.lit direction is entirely ignored by ToEE and replaced by a hardcoded sunlight direction
        // This is the normalized form of it:
        globalLight.dirX = -0.6324093645670703858428703903848;
        globalLight.dirY = -0.77463436252716949786709498111783;
        globalLight.dirZ = 0;

        if (globalLight.type != 3) {
            qWarning("Found a non-directional global light.");
        }

        zoneTemplate->setGlobalLight(globalLight);

        QHash<uint, QString> daylightEntries = MessageFile::parse(vfs->openFile("rules/daylight.mes"));

        // Try to find all the entries for our map
        QList<LightKeyframe> keyframesDay2d;
        QList<LightKeyframe> keyframesNight2d;
        QList<LightKeyframe> keyframesDay3d;
        QList<LightKeyframe> keyframesNight3d;

        readDaylightMes(daylightEntries, zoneTemplate->id() * 100, keyframesDay2d, keyframesNight2d);
        readDaylightMes(daylightEntries, zoneTemplate->id() * 100 + 24, keyframesDay3d, keyframesNight3d);

        uint nonEmptySets = keyframesDay2d.isEmpty() ? 0 : 1;
        nonEmptySets += keyframesDay3d.isEmpty() ? 0 : 1;
        nonEmptySets += keyframesNight2d.isEmpty() ? 0 : 1;
        nonEmptySets += keyframesNight3d.isEmpty() ? 0 : 1;

        if (nonEmptySets == 4) {
            zoneTemplate->setLightingKeyframesDay(keyframesDay2d, keyframesDay3d);
            zoneTemplate->setLightingKeyframesNight(keyframesNight2d, keyframesNight3d);
        } else if (nonEmptySets > 0) {
            qWarning("Map %d has %d non-empty daylight.mes sets. Should be either 4 or 0.", zoneTemplate->id(),
                     nonEmptySets);
        }

        return true;
    }

    bool ZoneTemplateReader::readSoundSchemes()
    {
        QByteArray mapInfo = vfs->openFile(zoneTemplate->directory() + "mapinfo.txt");

        QRegExp soundSchemePattern("SoundScheme: (\\d+),(\\d+)");

        QStringList lines = QString::fromLatin1(mapInfo).split("\n");

        foreach (QString line, lines) {
            line = line.trimmed();
            if (line.isEmpty())
                continue;
            if (!soundSchemePattern.exactMatch(line)) {
                qWarning("Unknown command in mapinfo.txt: %s for %s.", qPrintable(line),
                         qPrintable(zoneTemplate->directory()));
                continue;
            }

            uint primary = soundSchemePattern.cap(1).toUInt();
            uint secondary = soundSchemePattern.cap(2).toUInt();

            QList<uint> schemes;
            if (primary)
                schemes << primary;
            if (secondary)
                schemes << secondary;
            zoneTemplate->setSoundSchemes(schemes);
        }

        return true;
    }

}

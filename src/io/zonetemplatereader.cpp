
#include <QQuaternion>

#include "io/dagreader.h"
#include "io/messagefile.h"
#include "io/objectfilereader.h"
#include "io/skmreader.h"
#include "io/virtualfilesystem.h"
#include "io/zonetemplatereader.h"
#include "zonetemplate.h"
#include "zonebackgroundmap.h"
#include "geometrymeshobject.h"
#include "game.h"
#include "model.h"
#include "util.h"
#include "camera.h"
#include "models.h"
#include "clippinggeometryobject.h"

namespace EvilTemple
{

    const QString groundListFile("art/ground/ground.mes");

    ZoneTemplateReader::ZoneTemplateReader(const Game &game,
                                           ZoneTemplate *_zoneTemplate,
                                           const QString &_mapDirectory) :
        vfs(game.virtualFileSystem()),
        models(game.models()),
        prototypes(game.prototypes()),
        zoneTemplate(_zoneTemplate),
        mapDirectory(_mapDirectory)
    {
    }

    bool ZoneTemplateReader::read()
    {
        return readMapProperties()
                && readGroundDirectories()
                && readGeometryMeshFiles()
                && readGeometryMeshInstances()
                && readSectors()
                && readClippingMeshFiles()
                && readClippingMeshInstances();
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
            GeometryMeshObject *meshObject = new GeometryMeshObject(zoneTemplate);

            meshObject->setPosition(QVector3D(x, y, z));
            meshObject->setRotation(QQuaternion::fromAxisAndAngle(0, 1, 0, rotation));
            meshObject->setModelSource(new FileModelSource(models, geometryMeshFiles[fileIndex].modelFilename));

            zoneTemplate->addStaticGeometry(meshObject);
        }

        return true;
    }

    bool ZoneTemplateReader::readSectors()
    {
        QStringList sectorFiles = vfs->listFiles(mapDirectory, "*.sec");

        foreach (QString sector, sectorFiles)
        {
            readSector(sector);
        }

        return true;
    }

    bool ZoneTemplateReader::readSector(const QString &filename)
    {
        QByteArray data = vfs->openFile(filename);
        QDataStream stream(data);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        return readSectorLights(stream) && readSectorTiles(stream) && readSectorObjects(stream);
    }

    bool ZoneTemplateReader::readSectorLights(QDataStream &stream)
    {
        quint32 lightCount;
        stream >> lightCount;

        for (quint32 i = 0; i < lightCount; ++i)
        {
            quint32 flags;

            Light light;
            ParticleSystem particleSystem;

            stream >> light.handle >> flags >> light.type >> light.r >> light.b >> light.g >>
                    light.unknown >> light.ur >> light.ub >> light.ug >> light.ua >>
                    light.xPos >> light.yPos >> light.xOffset >> light.yOffset >> light.zOffset >>
                    light.dirX >> light.dirY >> light.dirZ >>
                    light.range >> light.phi;

            // Also check flags?
            if (light.type)
                lights.append(light);

            if ((flags & 0x10) != 0 || (flags & 0x40) != 0)
            {
                particleSystem.light = light;
                stream >> particleSystem.hash >> particleSystem.id;
                if (particleSystem.hash)
                    particleSystems.append(particleSystem);
            }

            if ((flags & 0x40) != 0)
            {
                stream >> light.type >> light.r >> light.b >> light.g >>
                        light.unknown >>
                        light.dirX >> light.dirY >> light.dirZ >>
                        light.range >> light.phi;

                // Also check flags?
                if (light.type)
                    lights.append(light);

                particleSystem.light = light;
                stream >> particleSystem.hash >> particleSystem.id;
                if (particleSystem.hash)
                    particleSystems.append(particleSystem);
            }
        }

        return true;
    }

    bool ZoneTemplateReader::readSectorTiles(QDataStream &stream)
    {
        SectorTile tiles[SectorSidelength][SectorSidelength];

        for (int y = 0; y < SectorSidelength; ++y)
        {
            for (int x = 0; x < SectorSidelength; ++x)
            {
                SectorTile &tile = tiles[x][y];
                stream >> tile.footstepsSound >> tile.unknown1[0] >> tile.unknown1[1] >> tile.unknown1[2]
                        >> tile.bitfield >> tile.unknown2;
            }
        }

        return true;
    }

    bool ZoneTemplateReader::readSectorObjects(QDataStream &stream)
    {
        int header;
        ObjectFileReader reader(prototypes, stream);

        stream.skipRawData(48); // Skip unknown data

        // Read objects while the object header is valid
        stream >> header;

        while (header == ObjectFileVersion)
        {
            if (!reader.read(true))
            {
                qWarning("Object file error: %s", qPrintable(reader.errorMessage()));
                return false;
            }

            zoneTemplate->addStaticGeometry(reader.createMeshObject(models));

            stream >> header;
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

            GeometryMeshObject *geometryMesh = new ClippingGeometryObject(zoneTemplate);

            // Create a model source for the DAG file
            // Please note: pre-creating would make more sense, but would entail using
            // a QSharedPointer in GeometryMeshObject instead of a QScopedPointer
            geometryMesh->setModelSource(new DagReader(vfs, clippingMeshFiles[meshIndex]));

            // Position the mesh correctly
            geometryMesh->setPosition(position);
            geometryMesh->setScale(scale);
            geometryMesh->setRotation(QQuaternion::fromAxisAndAngle(0, 1, 0, rad2deg(rotation)));

            zoneTemplate->addClippingGeometry(geometryMesh);
        }

        return true;
    }

}

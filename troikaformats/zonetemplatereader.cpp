
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
#include "model.h"
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
                && readGlobalLight();
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
        {
            readSector(sector);
        }

        return true;
    }

    bool ZoneTemplateReader::readMobiles()
    {
        QStringList mobFiles = vfs->listFiles(mapDirectory, "*.mob");

        QMap<QString, GameObject*> gameObjects;

        foreach (QString mobFile, mobFiles)
        {
            GameObject *gameObject = readMobile(mobFile);

            Q_ASSERT(!gameObject->id.isNull());

            gameObjects[gameObject->id] = gameObject;

            if (gameObject->parentItemId.isNull()) {
                zoneTemplate->addMobile(gameObject);
            }
        }

        foreach (GameObject *gameObject, gameObjects.values()) {
            if (gameObject->parentItemId.isNull())
                continue;

            Q_ASSERT(gameObjects.contains(gameObject->parentItemId));
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

        if (!reader.read(false)) {
            qWarning("Unable to read mobile file: %s", qPrintable(filename));
        }

        return new GameObject(reader.getObject());
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

        stream.skipRawData(48); // Skip unknown data

        // Read objects while the object header is valid
        stream >> header;

        while (header == ObjectFileVersion)
        {
            ObjectFileReader reader(prototypes, stream);

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

            float tmp = scale.z();
            scale.setZ(scale.y());
            scale.setY(tmp);

            float rotation;
            stream >> rotation;

            // TODO: The following transformation is incorrect and needs to be copied from the clipping geometry class

            GeometryObject *geometryMesh = new GeometryObject(position,
                                                              rad2deg(rotation),
                                                              scale,
                                                              clippingMeshFiles[meshIndex]);

            zoneTemplate->addClippingGeometry(geometryMesh);
        }

        return true;
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

        if (globalLight.type != 3) {
            qWarning("Found a non-directional global light.");
        }

        zoneTemplate->setGlobalLight(globalLight);
        return true;
    }

}

#ifndef ZONETEMPLATEREADER_H
#define ZONETEMPLATEREADER_H

#include "troikaformatsglobal.h"

#include <QSharedPointer>
#include <QString>
#include <QDataStream>
#include <QStringList>
#include <QVector>
#include <QHash>

#include "zonetemplate.h"

namespace Troika
{

    class VirtualFileSystem;
    class Game;
    class Models;
    class Prototypes;

    /**
      The side length of a square sector in tiles.
      */
    const int SectorSidelength = 64;

    struct GeometryMeshFile
    {
        QString modelFilename;
        QString animationFilename;
    };

    struct SectorTile
    {
        quint8 footstepsSound;
        quint8 unknown1[3];
        quint32 bitfield;
        quint64 unknown2;
    };

    class TROIKAFORMATS_EXPORT ZoneTemplateReader
    {
    public:
        ZoneTemplateReader(VirtualFileSystem *vfs,
                           Prototypes *prototypes,
                           ZoneTemplate *zoneTemplate, const QString &mapDirectory);

        bool read();

    private:

        /**
          The id of this map's background art in art/ground/ground.mes
          */
        quint32 artId;

        bool readMapProperties();
        bool readGroundDirectories();
        bool readGeometryMeshFiles(); // Loads gmesh.gmf
        bool readGeometryMeshInstances(); // Loads gmesh.gmi
        bool readSectors(); // Loads *.sec
        bool readSector(const QString &filename);
        bool readSectorLights(QDataStream &stream);
        bool readSectorTiles(QDataStream &stream);
        bool readSectorObjects(QDataStream &stream);
        bool readMobiles(); // Loads *.mob
        GameObject *readMobile(const QString &filename);
        bool readClippingMeshFiles(); // Loads clipping.cgf
        bool readClippingMeshInstances(); // Loads dag files + clipping.cif
		bool readGlobalLight(); // Loads global.lit

        VirtualFileSystem *vfs;
        Prototypes *prototypes;
        ZoneTemplate *zoneTemplate;        
        QString mapDirectory;
        QVector<GeometryMeshFile> geometryMeshFiles;
        QStringList clippingMeshFiles;
        QList<Light> lights;
        QList<ParticleSystem> particleSystems;
        QHash<uint,QString> meshMapping;
    };

}

#endif // ZONETEMPLATEREADER_H

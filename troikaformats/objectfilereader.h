#ifndef OBJECTFILEREADER_H
#define OBJECTFILEREADER_H

#include <gamemath.h>
using namespace GameMath;

#include "troikaformatsglobal.h"

#include "prototypes.h"

#include <QDataStream>
#include <QHash>
#include <QString>
#include <QVector3D>

namespace Troika
{

    class ObjectFileReaderData;
    class GeometryMeshObject;
    class VirtualFileSystem;
    class Materials;
    class Models;
    class GeometryObject;

    const int ObjectFileVersion = 0x77;

    /**
      Any object that has been placed on the map.
      */
    class GameObject {
    public:
        GameObject();
        ~GameObject();

        Integer strength;
        Integer dexterity;
        Integer constitution;
        Integer intelligence;
        Integer wisdom;
        Integer charisma;

        Bool dontDraw;

        Prototype *prototype;
        QString id; // object guid
        ObjectType objectType; // Must match prototype type
        QVector3D position; // Object position
        Integer name;
        Float scale; // (percent)
        Float rotation; // (degrees)
        Float radius;
        Float renderHeight;
        QStringList sceneryFlags;
        Integer descriptionId;
        QStringList secretDoorFlags;
        Integer secretDoorDc;
        QStringList portalFlags;
        Integer portalLockDc;
        Integer portalKeyId;
        QStringList flags;
        Integer teleportTarget;
        QString parentItemId;
        QString substituteInventoryId;
        Integer itemInventoryLocation;
        Integer hitPoints;
        Integer hitPointsDamage;
        Integer hitPointsAdjustment;
        Float walkSpeedFactor;
        Float runSpeedFactor;
        Integer dispatcher;
        Integer secretDoorEffect;
        Integer notifyNpc;
        QStringList containerFlags;
        Integer containerLockDc;
        Integer containerKeyId;
        Integer containerInventoryId;
        Integer containerInventoryListIndex;
        Integer containerInventorySource;
        QStringList itemFlags;
        Integer itemWeight;
        Integer itemWorth;
        Integer itemQuantity;
        QStringList weaponFlags;
        Integer ammoQuantity;
        QStringList armorFlags;
        Integer armorAcAdjustment;
        Integer armorMaxDexBonus;
        Integer armorCheckPenalty;
        Integer moneyQuantity;
        Integer keyId;
        QStringList critterFlags;
        QStringList critterFlags2;
        Integer critterRace;
        Integer critterGender;
        Integer critterMoneyIndex;
        Integer critterInventoryNum;
        Integer critterInventorySource;
        Bool locked;

        struct TeleportDestination
        {
            TeleportDestination() : defined(false) {}
            bool defined;
            uchar unknown;
            uint x;
            uint y;
        };
        TeleportDestination critterTeleportTo;
        Integer critterTeleportMap;
        Integer critterReach;
        Integer critterLevelUpScheme;
        QStringList npcFlags;
        Integer blitAlpha;
        Integer npcGeneratorData;
        Integer critterAlignment;

        QList<uint> factions;

        Integer standpointFlags;
        struct Standpoint
        {
            Standpoint() : defined(false) {}

            bool defined;
            uint map;
            uint flags;
            QVector3D position;
            int jumpPoint;
        };

        Standpoint dayStandpoint;
        Standpoint nightStandpoint;
        Standpoint scoutStandpoint;

        struct Waypoint
        {
            uint flags;
            QVector3D position;
            Float rotation;
            Integer delay;
            QList<uint> animations;
        };

        QList<Waypoint> waypoints;

        QList<GameObject*> content; // Based on parentitemid relation

    };

    class TROIKAFORMATS_EXPORT ObjectFileReader
    {
    public:
        ObjectFileReader(Prototypes *prototypes, QDataStream &stream);
        ~ObjectFileReader();

        bool read(bool skipHeader = false);
        const QString &errorMessage() const;

        const GameObject &getObject();

        GeometryObject *createObject(QHash<uint,QString> meshMapping);
    private:
        QScopedPointer<ObjectFileReaderData> d_ptr;

        Q_DISABLE_COPY(ObjectFileReader);
    };

}

#endif // OBJECTFILEREADER_H


#include <QUuid>
#include <QVector3D>
#include <QByteArray>
#include <QQuaternion>

#include "objectfilereader.h"
#include "skmreader.h"
#include "virtualfilesystem.h"
#include "messagefile.h"
#include "prototypes.h"
#include "util.h"
#include "zonetemplate.h"

namespace Troika
{

    const quint16 ObjectInstance = 1; // Type flag for instances
    const quint16 ObjectGuidEmbedded = 0; // Guid should be null
    const quint16 ObjectGuidMobile = 2; // Standard mobile guid

    // The length of the property bitfield for each object type in byte
    static int PropertyBlockSize[ObjectTypeCount] = {
        16, // Portal
        16, // Container
        20, // Scenery
        20, // Projectile
        28, // Weapon
        28, // Ammo
        32, // Armor
        32, // Money
        32, // Food
        32, // Scroll
        36, // Key
        36, // Written
        36, // Generic
        44, // Player Character
        52, // Non Player Character
        52, // Trap
        36 // Bag
    };

    enum ObjectProperty
    {
        Location = 0,
        OffsetX = 1,
        OffsetY = 2,
        BlitAlpha = 6,
        Scale = 7,
        Flags = 21,
        Unknown1 = 22,
        Name = 23,
        Description = 24,
        HpPts = 26,
        HpAdj = 27,
        HpDamage = 28,
        ScriptsIndex = 30,
        Rotation = 33,
        SpeedWalk = 34,
        SpeedRun = 35,
        Radius = 38,
        RenderHeight3d = 39,
        Conditions = 40,
        ConditionsArg0 = 41,
        PermanentMods = 42,
        Dispatcher = 44,
        SecretDoorFlags = 46,
        SecretDoorEffectName = 47,
        SecretDoorDc = 48,
        OffsetZ = 53,
        PermanentModData = 73,
        PortalFlags = 88,
        PortalLockDc = 89,
        PortalKeyId = 90,
        PortalNotifyNpc = 91,
        ContainerFlags = 102,
        ContainerLockDc = 103,
        ContainerKeyId = 104,
        ContainerInventoryNum = 105,
        ContainerInventoryListIndex = 106,
        ContainerInventorySource = 107,
        ContainerNotifyNpc = 108,
        SceneryFlags = 121,
        SceneryTeleportTo = 126,
        ItemFlags = 151,
        ItemParent = 152,
        ItemWeight = 153,
        ItemWorth = 154,
        ItemInventoryLocation = 156,
        ItemQuantity = 167,
        ItemPadWielderArgumentArray = 180,
        WeaponFlags = 187,
        AmmoQuantity = 210,
        ArmorFlags = 219,
        ArmoryACAdjustment = 220,
        ArmorMaxDexBonus = 221,
        ArmorArcaneSpellFailure = 222,
        ArmorArmorCheckPenalty = 223,
        MoneyQuantity = 230,
        KeyKeyId = 255,
        CritterFlags = 283,
        CritterFlags2 = 284,
        CritterAbilitiesIndex = 285,
        CritterRace = 287,
        CritterGender = 288,
        CritterPadi1 = 293,
        CritterAlignment = 294,
        CritterMoneyIndex = 307,
        CritterInventoryNum = 308,
        CritterInventoryListIndex = 309,
        CritterInventorySource = 310,
        CritterTeleportDestination = 313,
        CritterTeleportMap = 314,
        CritterReach = 317,
        CritterLevelUpScheme = 319,
        NpcFlags = 353,
        NpcWaypoints = 358,
        NpcStandpointDayInternal = 360,
        NpcStandpointNightInternal = 361,
        NpcFaction = 362,
        NpcSubstituteInventory = 364,
        NpcGeneratorData = 370,
        NpcAiFlags64 = 381,
        NpcStandpoints = 391,
    };

    class ObjectFileReaderData
    {
    public:
        Prototypes *prototypes;
        QDataStream &stream;
        QString errorMessage;

        quint32 prototypeId;
        QUuid guid;
        ObjectType objectType;
        Prototype *prototype;
        QVector3D position;

        bool customRotation;

        float scale;
        float rotation; // Degrees

        ObjectFileReaderData(QDataStream &_stream) : stream(_stream)
        {
            scale = 1.f;
            customRotation = false;
            rotation = 0;
        }

        /**
          Reads and validates the header of this object file.
          */
        bool validateHeader()
        {            
            int header;
            stream >> header;

            if (header != ObjectFileVersion)
            {
                errorMessage = QString("Invalid object header %1").arg(header);
                return false;
            }

            return true;
        }

        bool read()
        {
            if (!validateType())
            {
                return false;
            }

            stream.skipRawData(6); // Unknown short+int

            stream >> prototypeId;
            prototype = prototypes->get(prototypeId);
            if (prototype)
            {
                if (prototype->rotation.isDefined())
                    rotation = prototype->rotation.value(); // This is already in degrees
                if (prototype->scale.isDefined())
                    scale = prototype->scale.value();
                else
                    scale = 1;
            }

            stream.skipRawData(3 * sizeof(quint32));

            if (!readGuid())
                return false;

            quint32 objectTypeInt;
            stream >> objectTypeInt;
            objectType = (ObjectType)objectTypeInt;

            stream.skipRawData(sizeof(quint16)); // Unused, Some form of property count

            if (!readPropertyBlocks())
                return false;

            return true;
        }

        /**
          Reads the GUID type and the GUID itself.
          */
        bool readGuid()
        {
            quint16 guidType;

            stream >> guidType;

            if (guidType != ObjectGuidEmbedded && guidType != ObjectGuidMobile)
            {
                errorMessage = QString("Invalid guid type: %1").arg(guidType);
                return false;
            }

            stream.skipRawData(6); // Unknown bytes

            stream >> guid;

            return true;
        }

        /**
          Reads the variant property blocks.
          */
        bool readPropertyBlocks()
        {
            int blockSize = PropertyBlockSize[objectType];

            QByteArray propertyBlock(blockSize, Qt::Uninitialized);
            stream.readRawData(propertyBlock.data(), blockSize);

            // Iterate over each bit in the property block and read the
            // associated property
            int bitIndex = 0;
            for (int byteIndex = 0; byteIndex < blockSize; ++byteIndex)
            {
                quint8 propertyByte = propertyBlock[byteIndex];

                for (int bit = 0; bit < 8; ++bit)
                {
                    bool enabled = (propertyByte & (1 << bit)) != 0;

                    if (enabled)
                    {
                        readProperty((ObjectProperty)bitIndex);
                    }

                    bitIndex++;
                }
            }

            return true;
        }

        /**
          Reads a single optional property from the object file.
          */
        bool readProperty(ObjectProperty property)
        {
            quint32 x, y, ui32;
            float posOffset;

            switch (property)
            {
            case Location:
                stream.skipRawData(1); // Unused
                stream >> x >> y;

                position.setX((x + .5f) * PixelPerWorldTile);
                position.setZ((y + .5f) * PixelPerWorldTile);

                /*reader.ReadByte(); // Unused
                var x = reader.ReadUInt32(); // X
                var y = reader.ReadUInt32(); // Y
                mobile.Position += new Vector3((x + 0.5f)*GraphicsEngine.PixelPerTile,
                                               (y + 0.5f)*GraphicsEngine.PixelPerTile, mobile.Position.Z);*/
                //Console.WriteLine("Location: {0},{1}", x, y);
                break;
            case Scale:
                stream >> ui32;
                scale = ui32 / 100.f;
                break;
            case OffsetX:
                stream >> posOffset;
                position.setX(position.x() + posOffset);
                // mobile.Position += new Vector3(reader.ReadSingle(), 0, 0);
                break;
            case OffsetY:
                stream >> posOffset;
                position.setZ(position.z() + posOffset);
                // mobile.Position += new Vector3(0, reader.ReadSingle(), 0);
                break;
            case OffsetZ:
                stream >> posOffset;
                position.setY(position.y() + posOffset);
                // mobile.Position += new Vector3(0, 0, reader.ReadSingle());
                break;
            case Name:
                stream.skipRawData(4); // Index of some sort
                break;
            case SceneryFlags:
                stream.skipRawData(4);
                break;
            case Description:
                stream.skipRawData(4);
                break;
            case SecretDoorFlags:
                stream.skipRawData(4);
                break;
            case PortalFlags:
                stream.skipRawData(4);
                break;
            case SecretDoorDc:
                stream.skipRawData(4);
                break;
            case PortalLockDc:
                stream.skipRawData(4);
                break;
            case PortalKeyId:
                stream.skipRawData(4);
                break;
            case Flags:
                stream.skipRawData(4);
                //Console.WriteLine("Flags: {0}", flags);
                break;
            case Radius:
                stream.skipRawData(4); // Float
                //Console.WriteLine("Radius: {0}", radius);
                break;
            case RenderHeight3d:
                stream.skipRawData(4); // Float
                //Console.WriteLine("Radius: {0}", renderHeight);
                break;
            case SceneryTeleportTo:
                stream.skipRawData(4); // Teleport target
                //Console.WriteLine("Teleport Target: {0}", teleportTarget);
                break;
            case Rotation:
                stream >> rotation;
                customRotation = true;
                break;
            case Unknown1:
                stream.skipRawData(4);
                // Console.WriteLine(reader.ReadUInt32()); // Possibly spell flags or blocking stuff
                break;
            case ScriptsIndex:
                skipPropertyArray();
                break;
            case ItemParent:
                stream.skipRawData(1 + 8 + 16);
                // reader.ReadByte(); // GUID Type?
                // reader.ReadBytes(8); // Unknown/Unused, Probably same as in guid above
                // var guid = new Guid(reader.ReadBytes(16));
                break;
            case NpcSubstituteInventory:
                stream.skipRawData(1 + 8 + 16);
                // reader.ReadByte(); // GUID Type?
                // reader.ReadBytes(8); // Unknown/Unused, Probably same as in guid above
                // var guid2 = new Guid(reader.ReadBytes(16));
                break;
            case ItemInventoryLocation:
                stream.skipRawData(4);
                // reader.ReadUInt32();
                break;
            case Conditions:
                skipPropertyArray(); // Further structure unknown
                break;
            case HpPts:
                stream.skipRawData(4);
                //reader.ReadUInt32();
                break;
            case HpDamage:
                stream.skipRawData(4);
                //reader.ReadUInt32();
                break;
            case HpAdj:
                stream.skipRawData(4);
                //reader.ReadUInt32();
                break;
            case SpeedWalk:
                stream.skipRawData(4);
                // reader.ReadSingle();
                break;
            case SpeedRun:
                stream.skipRawData(4);
                // reader.ReadSingle();
                break;
            case ConditionsArg0:
                skipPropertyArray();
                break;
            case PermanentMods:
                skipPropertyArray();
                break;
            case Dispatcher:
                stream.skipRawData(4);
                // reader.ReadUInt32();
                break;
            case SecretDoorEffectName:
                stream.skipRawData(4);
                // reader.ReadUInt32();
                break;
            case PermanentModData:
                skipPropertyArray();
                break;
            case PortalNotifyNpc:
                stream.skipRawData(4);
                //reader.ReadUInt32();
                break;
            case ContainerFlags:
                stream.skipRawData(4);
                //reader.ReadUInt32();
                break;
            case ContainerLockDc:
                stream.skipRawData(4);
                //reader.ReadUInt32();
                break;
            case ContainerKeyId:
                stream.skipRawData(4);
                // reader.ReadUInt32();
                break;
            case ContainerInventoryNum:
                stream.skipRawData(4);
                // reader.ReadUInt32();
                break;
            case ContainerInventoryListIndex:
                stream.skipRawData(4);
                // reader.ReadUInt32();
                break;
            case ContainerInventorySource:
                stream.skipRawData(4);
                // reader.ReadUInt32();
                break;
            case ItemFlags:
                stream.skipRawData(4);
                // reader.ReadUInt32();
                break;
            case ItemWeight:
                stream.skipRawData(4);
                // reader.ReadUInt32();
                break;
            case ItemWorth:
                stream.skipRawData(4);
                // reader.ReadUInt32();
                break;
            case ItemQuantity:
                stream.skipRawData(4);
                // reader.ReadUInt32();
                break;
            case WeaponFlags:
                stream.skipRawData(4);
                // reader.ReadUInt32();
                break;
            case AmmoQuantity:
                stream.skipRawData(4);
                // reader.ReadUInt32();
                break;
            case ArmorFlags:
                stream.skipRawData(4);
                // reader.ReadUInt32();
                break;
            case ArmoryACAdjustment:
                stream.skipRawData(4);
                // reader.ReadInt32();
                break;
            case ArmorMaxDexBonus:
                stream.skipRawData(4);
                // reader.ReadInt32();
                break;
            case ArmorArmorCheckPenalty:
                stream.skipRawData(4);
                // reader.ReadInt32();
                break;
            case MoneyQuantity:
                stream.skipRawData(4);
                // reader.ReadUInt32();
                break;
            case KeyKeyId:
                stream.skipRawData(4);
                // reader.ReadUInt32();
                break;
            case CritterFlags:
                stream.skipRawData(4);
                // reader.ReadUInt32();
                break;
            case CritterFlags2:
                stream.skipRawData(4);
                // reader.ReadUInt32();
                break;
            case CritterRace:
                stream.skipRawData(4);
                //reader.ReadUInt32();
                break;
            case CritterGender:
                stream.skipRawData(4);
                // reader.ReadUInt32();
                break;
            case CritterMoneyIndex:
                stream.skipRawData(4);
                //reader.ReadUInt32();
                break;
            case CritterInventoryNum:
                stream.skipRawData(4);
                //reader.ReadUInt32();
                break;
            case CritterInventorySource:
                stream.skipRawData(4);
                //reader.ReadUInt32();
                break;
            case CritterTeleportDestination:
                stream.skipRawData(1 + 4 + 4);
                //reader.ReadByte();
                //reader.ReadInt32(); // x
                //reader.ReadInt32(); // y
                break;
            case CritterTeleportMap:
                stream.skipRawData(4);
                //reader.ReadUInt32();
                break;
            case CritterReach:
                stream.skipRawData(4);
                //reader.ReadUInt32();
                break;
            case CritterLevelUpScheme:
                stream.skipRawData(4);
                //reader.ReadUInt32();
                break;
            case NpcFlags:
                stream.skipRawData(4);
                //reader.ReadUInt32();
                break;
            case NpcStandpointDayInternal:
                stream.skipRawData(9);
                break;
            case NpcStandpointNightInternal:
                stream.skipRawData(9);
                // reader.ReadBytes(9);
                break;
            case BlitAlpha:
                stream.skipRawData(4);
                // reader.ReadUInt32();
                break;
            case NpcWaypoints:
                // LoadWaypoints(reader);
                skipPropertyArray();
                break;
            case NpcStandpoints:
                readStandpoints();
                break;
            case NpcFaction:
                skipPropertyArray();
                break;
            case CritterInventoryListIndex:
                skipPropertyArray();
                break;
            case NpcAiFlags64:
                stream.skipRawData(1 + 8);
                // reader.ReadByte(); // Version?
                // reader.ReadUInt64();
                break;
            case CritterPadi1:
                stream.skipRawData(4);
                // reader.ReadUInt32();
                break;
            case CritterAbilitiesIndex:
                skipPropertyArray();
                break;
            case NpcGeneratorData:
                stream.skipRawData(4);
                // reader.ReadUInt32(); // This information is packed.
                break;
            case CritterAlignment:
                stream.skipRawData(4);
                // reader.ReadUInt32(); // Format unknown
                break;
            case ItemPadWielderArgumentArray:
                skipPropertyArray();
                break;
            default:
                errorMessage = QString("Tried to read unknown property: %d").arg(property);
                return false;
            }

            return true;
        }

        void skipPropertyArray()
        {
            quint8 version;
            qint32 a, b, c, e, f;

            stream >> version;

            if (version == 0)
            {
                return; // This seems to mean, that the array is empty
            }

            stream >> a >> b >> c;

            // a*b: Taken from toee world builder
            QByteArray d(a * b, Qt::Uninitialized);
            stream.readRawData(d.data(), d.size());

            stream >> e;

            for (qint32 i = 0; i < e; ++i)
            {
                stream >> f;
            }
        }

        void readStandpoints()
        {
            quint32 SAR_POS_STN, dayMap, dayFlags, dayX, dayY, dayJP, nightMap, nightFlags, nightX, nightY, nightJP;
            float dayXOffset, dayYOffset, nightXOffset, nightYOffset;

            // Optional scout standpoint
            quint32 scoutMap, scoutFlags, scoutX, scoutY, scoutJP;
            float scoutXOffset, scoutYOffset;

            // Skip the pre-struct
            stream.skipRawData(1 + 4);
            quint32 type;
            stream >> type;

            // Load the SARC thingie
            stream >> SAR_POS_STN; // SAR_POS_STN

            // Load standpoints
            stream >> dayMap >> dayFlags >> dayX >> dayY >> dayXOffset >> dayYOffset >> dayJP;
            stream.skipRawData(52);

            stream >> nightMap >> nightFlags >> nightX >> nightY >> nightXOffset >> nightYOffset >> nightJP;
            stream.skipRawData(52);

            if (type == 0x1E)
            {
                stream >> scoutMap >> scoutFlags >> scoutX >> scoutY >> scoutXOffset >> scoutYOffset >> scoutJP;
                stream.skipRawData(52);
            }

            // Skip the post-struct
            stream.skipRawData(12);
        }

        /**
          Reads and validates the object type (which must be an object instance).
          */
        bool validateType()
        {
            quint16 type;
            stream >> type;

            if (type != ObjectInstance)
            {
                errorMessage = QString("Invalid object type: %1").arg(type);
                return false;
            }

            return true;
        }

        GeometryObject *createMeshObject(QHash<uint,QString> meshMapping)
        {
            /*GeometryMeshObject *result = new GeometryMeshObject();

            result->setPosition(position);
            result->setRotation(QQuaternion::fromAxisAndAngle(0, 1, 0, rotation));
            result->setScale(QVector3D(scale, scale, scale));
            result->setModelSource(new LegacyModelSource(models, prototype->modelId()));*/

            GeometryObject *obj = new GeometryObject(position,
                                                     QQuaternion::fromAxisAndAngle(0, 1, 0, rad2deg(rotation + LegacyBaseRotation)),
                                                     QVector3D(scale, scale, scale),
                                                     meshMapping[prototype->modelId]);
            return obj;
        }
    };

    ObjectFileReader::ObjectFileReader(Prototypes *prototypes, QDataStream &stream) :
            d_ptr(new ObjectFileReaderData(stream))
    {
        d_ptr->prototypes = prototypes;
    }

    ObjectFileReader::~ObjectFileReader()
    {
    }

    const QString &ObjectFileReader::errorMessage() const
    {
        return d_ptr->errorMessage;
    }

    bool ObjectFileReader::read(bool skipHeader)
    {
        if (!skipHeader && !d_ptr->validateHeader())
            return false;

        return d_ptr->read();
    }

    GeometryObject *ObjectFileReader::createObject(QHash<uint,QString> meshMapping)
    {
        return d_ptr->createMeshObject(meshMapping);
    }

}

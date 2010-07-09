
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

    static const QString ObjectFlagNames[32] = {
        "Destroyed", // Not used
        "Off", // Used
        "Flat", // Isn't this better handled by the prototype?
        "Text",
        "SeeThrough",
        "ShootThrough",
        "Translucent",
        "Shrunk",
        "DontDraw",
        "Invisible",
        "NoBlock",
        "ClickThrough",
        "", // Inventory. Now induced implicitly
        "Dynamic",
        "ProvidesCover",
        "RandomSize",
        "NoHeight",
        "Wading",
        "Unknown18",
        "Stoned",
        "DontLight",
        "", // TextFloater, purpose unknown
        "Invulnerable",
        "Extinct",
        "TrapPc",
        "TrapSpotted",
        "DisallowWading",
        "Unknown27",
        "", // HeightSet
        "AnimatedDead",
        "Teleported",
        "" // RadiusSet
    };

    static const QString SceneryFlagNames[32] = {
        "NoAutoAnimate",
        "Busted",
        "Nocturnal",
        "MarksTownmap",
        "IsFire",
        "Respawnable",
        "SoundSmall",
        "SoundMedium",
        "SoundExtraLarge",
        "UnderAll",
        "Respawning",
        "TaggedScenery",
        "UseOpenWorldmap",
        "Unknown13",
        "Unknown14",
        "Unknown15",
        "Unknown16",
        "Unknown17",
        "Unknown18",
        "Unknown19",
        "Unknown20",
        "Unknown21",
        "Unknown22",
        "Unknown23",
        "Unknown24",
        "Unknown25",
        "Unknown26",
        "Unknown27",
        "Unknown28",
        "Unknown29",
        "Unknown30",
        "Unknown31"
    };

    static const QString ContainerFlagNames[32] = {
        "Locked",
        "Jammed", // Unused
        "MagicallyHeld", // Unused
        "NeverLocked",
        "AlwaysLocked", // Unused
        "LockedDay", // Unused
        "LockedNight", // Unused
        "Busted", // Unused
        "NotSticky", // Unused
        "InvenSpaceOnce", // Unused
        "InvenSpaceIndependent", // Unused
        "Open", // Unused
        "HasBeenOpened", // Unused
        "Unknown13",
        "Unknown14",
        "Unknown15",
        "Unknown16",
        "Unknown17",
        "Unknown18",
        "Unknown19",
        "Unknown20",
        "Unknown21",
        "Unknown22",
        "Unknown23",
        "Unknown24",
        "Unknown25",
        "Unknown26",
        "Unknown27",
        "Unknown28",
        "Unknown29",
        "Unknown30",
        "Unknown31"
    };

    static const QString PortalFlagNames[32] = {
        "Locked",
        "Jammed",
        "MagicallyHeld",
        "NeverLocked",
        "AlwaysLocked",
        "LockedDay",
        "LockedNight",
        "Busted",
        "NotSticky",
        "Open",
        "Unknown10",
        "Unknown11",
        "Unknown12",
        "Unknown13",
        "Unknown14",
        "Unknown15",
        "Unknown16",
        "Unknown17",
        "Unknown18",
        "Unknown19",
        "Unknown20",
        "Unknown21",
        "Unknown22",
        "Unknown23",
        "Unknown24",
        "Unknown25",
        "Unknown26",
        "Unknown27",
        "Unknown28",
        "Unknown29",
        "Unknown30",
        "Unknown31"
    };

    static const QString SecretDoorFlagNames[32] = {
        "Unknown0",
        "Unknown1",
        "Unknown2",
        "Unknown3",
        "Unknown4",
        "Unknown5",
        "Unknown6",
        "Unknown7",
        "Unknown8",
        "Unknown9",
        "Unknown10",
        "Unknown11",
        "Unknown12",
        "Unknown13",
        "Unknown14",
        "Unknown15",
        "Unknown16",
        "Unknown17",
        "Unknown18",
        "Unknown19",
        "Unknown20",
        "Unknown21",
        "Unknown22",
        "Unknown23",
        "Unknown24",
        "Unknown25",
        "Unknown26",
        "Unknown27",
        "Unknown28",
        "Unknown29",
        "Unknown30",
        "Unknown31"
    };

    static const QString ItemFlagNames[32] = {
        "Identified",
        "WontSell",
        "IsMagical",
        "NoPickPocket",
        "NoDisplay",
        "NoDrop",
        "NeedsSpell",
        "CanUseBox",
        "NeedsTarget",
        "LightSmall",
        "LightMedium",
        "LightLarge",
        "LightExtraLarge",
        "Persistent",
        "MtTriggered",
        "Stolen",
        "UseIsThrown",
        "NoDecay",
        "Uber",
        "NoNpcPickup",
        "NoRangedUse",
        "ValidAiAction",
        "DrawWhenParented",
        "ExpiresAfterUse",
        "NoLoot",
        "UsesWandAnim",
        "NoTransfer",
        "Unknown27",
        "Unknown28",
        "Unknown29",
        "Unknown30",
        "Unknown31"
    };

    static const QString WeaponFlagNames[32] = {
        "Loud",
        "Silent",
        "OWF_UNUSED_1",
        "OWF_UNUSED_2",
        "Throwable",
        "TransProjectile",
        "Boomerangs",
        "IgnorePersistence",
        "DamageArmor",
        "DefaultThrows",
        "RangedWeapon",
        "WeaponLoaded",
        "MagicStaff",
        "Unknown13",
        "Unknown14",
        "Unknown15",
        "Unknown16",
        "Unknown17",
        "Unknown18",
        "Unknown19",
        "Unknown20",
        "Unknown21",
        "Unknown22",
        "Unknown23",
        "Unknown24",
        "Unknown25",
        "Unknown26",
        "Unknown27",
        "Unknown28",
        "Unknown29",
        "Unknown30",
        "Unknown31"
    };

    static const QString ArmorFlagNames[32] = {
        "ArmorType1",
        "ArmorType2",
        "HelmType1",
        "HelmType2",
        "ArmorNone",
        "Unknown5",
        "Unknown6",
        "Unknown7",
        "Unknown8",
        "Unknown9",
        "Unknown10",
        "Unknown11",
        "Unknown12",
        "Unknown13",
        "Unknown14",
        "Unknown15",
        "Unknown16",
        "Unknown17",
        "Unknown18",
        "Unknown19",
        "Unknown20",
        "Unknown21",
        "Unknown22",
        "Unknown23",
        "Unknown24",
        "Unknown25",
        "Unknown26",
        "Unknown27",
        "Unknown28",
        "Unknown29",
        "Unknown30",
        "Unknown31"
    };

    static const QString CritterFlagNames[32] = {
        "IsConcealed",
        "MovingSilently",
        "ExperienceAwarded",
        "Unused00000008",
        "Fleeing",
        "Stunned",
        "Paralyzed",
        "Blinded",
        "HasArcaneAbility",
        "Unused00000200",
        "Unused00000400",
        "Unused00000800",
        "Sleeping",
        "Mute",
        "Surrendered",
        "Monster",
        "SpellFlee",
        "Encounter",
        "CombatModeActive",
        "LightSmall",
        "LightMedium",
        "LightLarge",
        "LightExtraLarge",
        "Unrevivifiable",
        "Unresurrectable",
        "Unused02000000",
        "Unused04000000",
        "NoFlee",
        "NonLethalCombat",
        "Mechanical",
        "Unused40000000",
        "FatigueLimiting"
    };

    static const QString CritterFlagNames2[32] = {
        "Unknown0",
        "Unknown1",
        "Unknown2",
        "Unknown3",
        "Unknown4",
        "Unknown5",
        "Unknown6",
        "Unknown7",
        "Unknown8",
        "Unknown9",
        "Unknown10",
        "Unknown11",
        "Unknown12",
        "Unknown13",
        "Unknown14",
        "Unknown15",
        "Unknown16",
        "Unknown17",
        "Unknown18",
        "Unknown19",
        "Unknown20",
        "Unknown21",
        "Unknown22",
        "Unknown23",
        "Unknown24",
        "Unknown25",
        "Unknown26",
        "Unknown27",
        "Unknown28",
        "Unknown29",
        "Unknown30",
        "Unknown31"
    };

    static const QString NpcFlagNames[32] = {
        "ExFollower",
        "WaypointsDay",
        "WaypointsNight",
        "AiWaitHere",
        "AiSpreadOut",
        "Jilted",
        "LogbookIgnores",
        "ONF_UNUSED_00000080",
        "KillOnSight",
        "UseAlertPoints",
        "ForcedFollower",
        "KillOnSightOverride",
        "Wanders",
        "WandersInDark",
        "Fence",
        "Familiar",
        "CheckLeader",
        "NoEquip",
        "CastHighest",
        "Generator",
        "Generated",
        "GeneratorRate1",
        "GeneratorRate2",
        "GeneratorRate3",
        "DemaintainSpells",
        "ONF_UNUSED_02000000",
        "ONF_UNUSED_04000000",
        "ONF_UNUSED_08000000",
        "BackingOff",
        "NoAttack",
        "BossMonster",
        "Extraplanar"
    };

    static bool equalFlags(const QStringList &a, const QStringList &b)
    {
        if (a.size() != b.size())
            return false;

        foreach (const QString &elem, a) {
            if (!b.contains(elem))
                return false;
        }

        return true;
    }

    static QVector3D getPosition(uint x, uint y, float xOffset, float yOffset)
    {
        return QVector3D((x + 0.5f) * PixelPerWorldTile + xOffset, 0, (y + 0.5f) * PixelPerWorldTile + yOffset);
    }

    GameObject::GameObject() : prototype(0)
    {
    }

    GameObject::~GameObject()
    {
        qDeleteAll(content);
    }

    template<typename T>
    inline QDataStream &operator >>(QDataStream &stream, Property<T> &property)
    {
        T value;
        stream >> value;
        property.setValue(value);
        return stream;
    }

    class ObjectFileReaderData
    {
    public:
        Prototypes *prototypes;
        QDataStream &stream;
        QString errorMessage;

        GameObject object;

        ObjectFileReaderData(QDataStream &_stream) : stream(_stream)
        {
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

            uint prototypeId;
            stream >> prototypeId;
            object.prototype = prototypes->get(prototypeId);
            Q_ASSERT_X(object.prototype, "ObjectFileReader::read", qPrintable(QString("%1").arg(prototypeId)));

            stream.skipRawData(3 * sizeof(quint32));

            if (!readGuid())
                return false;

            quint32 objectTypeInt;
            stream >> objectTypeInt;
            object.objectType = (ObjectType)objectTypeInt;

            Q_ASSERT(object.prototype->type == object.objectType);

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

            QUuid guid;
            stream >> guid;
            if (guidType == ObjectGuidMobile)
                object.id = guid.toString();

            return true;
        }

        /**
          Reads the variant property blocks.
          */
        bool readPropertyBlocks()
        {
            int blockSize = PropertyBlockSize[object.objectType];

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

        void convertFlags(const QString *flagNames, uint flags, QStringList &flagList)
        {
            for (int i = 0; i < 32; ++i) {
                bool set = ((flags >> i) & 1) == 1;
                if (set && !flagNames[i].isEmpty()) {
                    flagList.append(flagNames[i]);
                }
            }
        }

        /**
          Reads a single optional property from the object file.
          */
        bool readProperty(ObjectProperty property)
        {
            quint32 x, y, ui32;
            float posOffset;
            QUuid guid;
            uint flags;

            switch (property)
            {
            case Location:
                stream.skipRawData(1); // Unused
                stream >> x >> y;

                object.position.setX((x + .5f) * PixelPerWorldTile);
                object.position.setZ((y + .5f) * PixelPerWorldTile);
                break;
            case Scale:
                stream >> ui32;
                object.scale.setValue(ui32);
                if (object.scale == object.prototype->scale)
                    object.scale.clear();
                break;
            case OffsetX:
                stream >> posOffset;
                object.position.setX(object.position.x() + posOffset);
                break;
            case OffsetY:
                stream >> posOffset;
                object.position.setZ(object.position.z() + posOffset);
                break;
            case OffsetZ:
                stream >> posOffset;
                object.position.setY(object.position.y() + posOffset);
                break;
            case Name:
                stream >> object.name;
                break;
            case SceneryFlags:
                stream >> flags;
                convertFlags(SceneryFlagNames, flags, object.sceneryFlags);
                if (object.sceneryFlags.toSet() == qobject_cast<SceneryProperties*>(object.prototype->additionalProperties)->flags.toSet())
                    object.sceneryFlags.clear();
                break;
            case Description:
                stream >> object.descriptionId;
                if (object.descriptionId.value() == object.prototype->descriptionId)
                    object.descriptionId.clear();
                break;
            case SecretDoorFlags:
                stream >> flags;
                convertFlags(SecretDoorFlagNames, flags, object.secretDoorFlags);
                break;
            case PortalFlags:
                stream >> flags;
                convertFlags(PortalFlagNames, flags, object.portalFlags);
                if (object.portalFlags.toSet() == qobject_cast<PortalProperties*>(object.prototype->additionalProperties)->flags.toSet())
                    object.portalFlags.clear();
                break;
            case SecretDoorDc:
                stream >> object.secretDoorDc;
                break;
            case PortalLockDc:
                stream >> object.portalLockDc;
                if (object.portalLockDc == qobject_cast<PortalProperties*>(object.prototype->additionalProperties)->lockDc)
                    object.portalLockDc.clear();
                break;
            case PortalKeyId:
                stream >> object.portalKeyId;
                if (object.portalKeyId == qobject_cast<PortalProperties*>(object.prototype->additionalProperties)->keyId)
                    object.portalKeyId.clear();
                break;
            case Flags:
                stream >> flags;
                convertFlags(ObjectFlagNames, flags, object.flags);
                if (object.flags.removeAll("DontDraw") > 0 && !object.prototype->dontDraw.isDefined())
                    object.dontDraw.setValue(true);
                if (object.flags.toSet() == object.prototype->objectFlags.toSet())
                    object.flags.clear();
                break;
            case Radius:
                stream >> object.radius;
                // the first object that is encountered sets the prototype's radius
                if (!object.prototype->radius.isDefined())
                    object.prototype->radius.setValue(object.radius.value());
                if (object.radius == object.prototype->radius)
                    object.radius.clear();
                break;
            case RenderHeight3d:
                stream >> object.renderHeight;
                // the first object that is encountered sets the prototype's render height
                if (!object.prototype->renderHeight.isDefined())
                    object.prototype->renderHeight.setValue(object.renderHeight.value());
                if (object.renderHeight == object.prototype->renderHeight)
                    object.renderHeight.clear();
                break;
            case SceneryTeleportTo:
                stream >> object.teleportTarget;
                break;
            case Rotation:
                stream >> object.rotation;
                object.rotation.setValue(rad2deg(LegacyBaseRotation + object.rotation.value()));
                if (object.rotation == object.prototype->rotation)
                    object.rotation.clear();
                break;
            case Unknown1:
                stream.skipRawData(4);
                // Console.WriteLine(reader.ReadUInt32()); // Possibly spell flags or blocking stuff
                break;
            case ScriptsIndex:
                skipPropertyArray();
                break;
            case ItemParent:
                stream.skipRawData(1 + 8);
                stream >> guid;
                if (guid.isNull()) {
                    qWarning("Null GUID read from file.");
                } else {
                    object.parentItemId = guid.toString();
                }
                break;
            case NpcSubstituteInventory:
                stream.skipRawData(1 + 8);
                stream >> guid;
                object.substituteInventoryId = guid.toString();
                break;
            case ItemInventoryLocation:
                stream >> object.itemInventoryLocation;
                break;
            case Conditions:
                skipPropertyArray(); // Further structure unknown
                break;
            case HpPts:
                stream >> object.hitPoints;
                if (object.hitPoints == object.prototype->hitPoints)
                    object.hitPoints.clear();
                break;
            case HpDamage:
                stream >> object.hitPointsDamage;
                break;
            case HpAdj:
                stream >> object.hitPointsAdjustment;
                break;
            case SpeedWalk:
                stream >> object.walkSpeedFactor;
                if (object.walkSpeedFactor == object.prototype->walkSpeedFactor)
                    object.walkSpeedFactor.clear();
                break;
            case SpeedRun:
                stream >> object.runSpeedFactor;
                if (object.runSpeedFactor == object.prototype->runSpeedFactor)
                    object.runSpeedFactor.clear();
                break;
            case ConditionsArg0:
                skipPropertyArray();
                break;
            case PermanentMods:
                skipPropertyArray();
                break;
            case Dispatcher:
                stream >> object.dispatcher;
                if (object.dispatcher.value() == -1)
                    object.dispatcher.clear();
                break;
            case SecretDoorEffectName:
                stream >> object.secretDoorEffect; // Hashed particle system name?
                break;
            case PermanentModData:
                skipPropertyArray();
                break;
            case PortalNotifyNpc:
                stream >> object.notifyNpc;
                break;
            case ContainerFlags:
                stream >> flags;
                convertFlags(ContainerFlagNames, flags, object.containerFlags);
                if (object.containerFlags.contains("Locked"))
                    object.locked.setValue(true);
                else if (object.containerFlags.contains("NeverLocked"))
                    object.locked.setValue(false);
                if (object.locked.value() == qobject_cast<ContainerProperties*>(object.prototype->additionalProperties)->locked)
                    object.locked.clear();
                break;
            case ContainerLockDc:
                stream >> object.containerLockDc;
                break;
            case ContainerKeyId:
                stream >> object.containerKeyId;
                break;
            case ContainerInventoryNum:
                stream >> object.containerInventoryId;
                break;
            case ContainerInventoryListIndex:
                stream >> object.containerInventoryListIndex;
                break;
            case ContainerInventorySource:
                stream >> object.containerInventorySource;
                break;
            case ItemFlags:
                stream >> flags;
                convertFlags(ItemFlagNames, flags, object.itemFlags);
                if (object.itemFlags.toSet() == qobject_cast<ItemProperties*>(object.prototype->additionalProperties)->flags.toSet())
                    object.itemFlags.clear();
                break;
            case ItemWeight:
                stream >> object.itemWeight;
                break;
            case ItemWorth:
                stream >> object.itemWorth;
                break;
            case ItemQuantity:
                stream >> object.itemQuantity;
                break;
            case WeaponFlags:
                stream >> flags;
                convertFlags(WeaponFlagNames, flags, object.weaponFlags);
                if (object.weaponFlags.toSet() == qobject_cast<WeaponProperties*>(object.prototype->additionalProperties)->flags.toSet())
                    object.weaponFlags.clear();
                break;
            case AmmoQuantity:
                stream >> object.ammoQuantity;
                break;
            case ArmorFlags:
                stream >> flags;
                convertFlags(ArmorFlagNames, flags, object.armorFlags);
                if (object.armorFlags.toSet() == qobject_cast<ArmorProperties*>(object.prototype->additionalProperties)->flags.toSet())
                    object.armorFlags.clear();
                break;
            case ArmoryACAdjustment:
                stream >> object.armorAcAdjustment;
                break;
            case ArmorMaxDexBonus:
                stream >> object.armorMaxDexBonus;
                break;
            case ArmorArmorCheckPenalty:
                stream >> object.armorCheckPenalty;
                break;
            case MoneyQuantity:
                stream >> object.moneyQuantity;
                break;
            case KeyKeyId:
                stream >> object.keyId;
                break;
            case CritterFlags:
                stream >> flags;
                convertFlags(CritterFlagNames, flags, object.critterFlags);
                if (object.critterFlags.toSet() == qobject_cast<CritterProperties*>(object.prototype->additionalProperties)->flags.toSet())
                    object.critterFlags.clear();
                break;
            case CritterFlags2:
                stream >> flags;
                convertFlags(CritterFlagNames2, flags, object.critterFlags2);
                break;
            case CritterRace:
                stream >> object.critterRace;
                break;
            case CritterGender:
                stream >> object.critterGender;
                break;
            case CritterMoneyIndex:
                //stream >> object.critterMoneyIndex;
                skipPropertyArray();
                break;
            case CritterInventoryNum:
                stream >> object.critterInventoryNum;
                break;
            case CritterInventorySource:
                stream >> object.critterInventorySource;
                break;
            case CritterTeleportDestination:
                stream >> object.critterTeleportTo.unknown >> object.critterTeleportTo.x >> object.critterTeleportTo.y;
                object.critterTeleportTo.defined = true;
                break;
            case CritterTeleportMap:
                stream >> object.critterTeleportMap;
                break;
            case CritterReach:
                stream >> object.critterReach;
                break;
            case CritterLevelUpScheme:
                stream >> object.critterLevelUpScheme;
                break;
            case NpcFlags:
                stream >> flags;
                convertFlags(NpcFlagNames, flags, object.npcFlags);
                if (object.npcFlags.toSet() == qobject_cast<NonPlayerCharacterProperties*>(object.prototype->additionalProperties)->flags.toSet())
                    object.npcFlags.clear();
                break;
            case NpcStandpointDayInternal:
                stream.skipRawData(9);
                break;
            case NpcStandpointNightInternal:
                stream.skipRawData(9);
                break;
            case BlitAlpha:
                stream >> object.blitAlpha;
                break;
            case NpcWaypoints:
                readWaypoints();
                break;
            case NpcStandpoints:
                readStandpoints();
                break;
            case NpcFaction:
                readFactions();
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
                                readAbilities();
                break;
            case NpcGeneratorData:
                stream >> object.npcGeneratorData; // This information is packed.
                break;
            case CritterAlignment:
                stream >> object.critterAlignment; // Format unknown
                // Used for only 2 NPCs overall, ignore it for now
                // qDebug("Critter-Alignment: %d", object.critterAlignment.value());
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

        void readAbilities()
        {
            uchar version;
            uint recordSize, recordCount, structureId;

            stream >> version >> recordSize >> recordCount >> structureId;

            Q_ASSERT(version == 1);
            Q_ASSERT(recordSize == 4);

            stream >> object.strength >> object.dexterity >> object.constitution >> object.intelligence
                    >> object.wisdom >> object.charisma;

            /*
             To use as little data as possible per map, reset every value that corresponds with the prototype.
             */
            CritterProperties *critterProps = qobject_cast<CritterProperties*>(object.prototype->additionalProperties);
            Q_ASSERT(critterProps);
            if (object.strength.value() == critterProps->strength)
                object.strength.clear();
            if (object.dexterity.value() == critterProps->dexterity)
                object.dexterity.clear();
            if (object.constitution.value() == critterProps->constitution)
                object.constitution.clear();
            if (object.intelligence.value() == critterProps->intelligence)
                object.intelligence.clear();
            if (object.wisdom.value() == critterProps->wisdom)
                object.wisdom.clear();
            if (object.charisma.value() == critterProps->charisma)
                object.charisma.clear();

            uint trailingNumberCount;
            stream >> trailingNumberCount;
            stream.skipRawData(sizeof(uint) * trailingNumberCount);
        }

        void readWaypoints()
        {
            uchar version;
            uint recordSize, recordCount, structureId, waypointCount;

            stream >> version;
            Q_ASSERT(version == 1);
            stream >> recordSize >> recordCount >> structureId >> waypointCount;

            stream.skipRawData(12); // Unused

            for (uint i = 0; i < waypointCount; ++i) {
                GameObject::Waypoint waypoint;

                uint flags, x, y, delay;
                uchar animation;
                float rotation, xOffset, yOffset;

                stream >> flags >> x >> y >> xOffset >> yOffset >> rotation;

                waypoint.position = getPosition(x, y, xOffset, yOffset);
                if (flags & 1)
                    waypoint.rotation.setValue(rotation);
                for (int i = 0; i < 8; ++i) {
                    stream >> animation;
                    if (flags & 4 && animation > 0)
                        waypoint.animations.append(animation);
                }

                stream >> delay;
                stream.skipRawData(sizeof(uint) * 7);

                if (flags & 2)
                    waypoint.delay.setValue(delay);

                object.waypoints.append(waypoint);
            }

            // The structure seems to be padded, skip rest
            stream.skipRawData(qMax<int>(0, recordSize * recordCount - waypointCount * 64 - 16));

            uint trailingNumberCount;
            stream >> trailingNumberCount;
            stream.skipRawData(sizeof(uint) * trailingNumberCount);
        }

        void readFactions()
        {
            uchar version;
            uint recordSize, recordCount, structureId;

            stream >> version >> recordSize >> recordCount >> structureId;

            Q_ASSERT(version == 1);
            Q_ASSERT(recordSize == 4);

            uint faction;

            for (uint i = 0; i < recordCount; ++i) {
                stream >> faction;
                object.factions.append(faction);
            }

            uint trailingNumberCount;
            stream >> trailingNumberCount;
            stream.skipRawData(sizeof(uint) * trailingNumberCount);
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
            stream >> object.standpointFlags;
            quint32 type = object.standpointFlags.value();

            // Load the SARC thingie
            stream >> SAR_POS_STN; // SAR_POS_STN

            // Load standpoints
            stream >> dayMap >> dayFlags >> dayX >> dayY >> dayXOffset >> dayYOffset >> dayJP;
            stream.skipRawData(52);
            if (type & 0x14) {
                object.dayStandpoint.defined = true;
                object.dayStandpoint.flags = dayFlags;
                object.dayStandpoint.jumpPoint = dayJP;
                object.dayStandpoint.map = dayMap;
                object.dayStandpoint.position = getPosition(dayX, dayY, dayXOffset, dayYOffset);
            }

            stream >> nightMap >> nightFlags >> nightX >> nightY >> nightXOffset >> nightYOffset >> nightJP;
            stream.skipRawData(52);
            if (type & 0x14) {
                object.nightStandpoint.defined = true;
                object.nightStandpoint.flags = nightFlags;
                object.nightStandpoint.jumpPoint = nightJP;
                object.nightStandpoint.map = nightMap;
                object.nightStandpoint.position = getPosition(nightX, nightY, nightXOffset, nightYOffset);
            }

            if (type == 0x1E)
            {
                stream >> scoutMap >> scoutFlags >> scoutX >> scoutY >> scoutXOffset >> scoutYOffset >> scoutJP;
                stream.skipRawData(52);
                object.scoutStandpoint.defined = true;
                object.scoutStandpoint.flags = scoutFlags;
                object.scoutStandpoint.jumpPoint = scoutJP;
                object.scoutStandpoint.map = scoutMap;
                object.scoutStandpoint.position = getPosition(scoutX, scoutY, scoutXOffset, scoutYOffset);
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

            /*GeometryObject *obj = new GeometryObject(object.position,
                                                     QQuaternion::fromAxisAndAngle(0, 1, 0, rad2deg(rotation + LegacyBaseRotation)),
                                                     QVector3D(scale, scale, scale),
                                                     meshMapping[prototype->modelId]);*/
            return NULL;
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

    const GameObject &ObjectFileReader::getObject()
    {
        return d_ptr->object;
    }

}

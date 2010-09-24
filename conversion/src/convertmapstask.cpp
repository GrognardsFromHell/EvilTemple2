
#include <qbox3d.h>

#include <serializer.h>
using namespace QJson;

#include <constants.h>
#include <zonetemplates.h>
#include <zonebackgroundmap.h>
#include <messagefile.h>
#include <dagreader.h>
#include <virtualfilesystem.h>
#include <troika_model.h>

#include "conversion/convertmapstask.h"
#include "conversion/mapareamapping.h"
#include "conversion/mapconverter.h"
#include "conversion/pathnodeconverter.h"
#include "conversion/util.h"
#include "conversion/sectorconverter.h"
#include "conversion/clippingmeshconverter.h"

#include <QDir>

static QVariantMap waypointToMap(const GameObject::Waypoint &waypoint)
{
    QVariantMap result;

    result["position"] = vectorToList(waypoint.position);
    if (waypoint.rotation.isDefined())
        result["rotation"] = waypoint.rotation.value();
    if (waypoint.delay.isDefined())
        result["delay"] = waypoint.delay.value();

    if (!waypoint.animations.isEmpty()) {
        QVariantList animations;
        foreach (uint anim, waypoint.animations)
            animations.append(anim);
        result["animations"] = animations;
    }

    return result;
}

static QVariantMap standpointToMap(IConversionService *service, const GameObject::Standpoint &standpoint)
{
    QVariantMap result;

    if (standpoint.defined) {
        /*
          There are some broken standpoints in the vanilla files with NAN x/y coordinates.
          They do have map=0 && jumpPoint == 0, so we use that as an exclusion criteria here.
          */
        if (standpoint.jumpPoint != -1 && standpoint.jumpPoint != 0) {
            result["jumpPoint"] = standpoint.jumpPoint;
        } else if (standpoint.map != 0) {
            result["map"] = service->convertMapId(standpoint.map);
            result["position"] = vectorToList(standpoint.position);
        }
    }

    return result;
}

static QVariantList factionsToList(const QList<uint> &factions)
{
    QVariantList result;

    foreach (uint faction, factions) {
        // I think including the 0 faction is pointless
        if (faction != 0)
            result.append(faction);
    }

    return result;
}

static QVariant toVariant(IConversionService *service, GameObject *object, QVariantMap *parent = NULL)
{
    QVariantMap objectMap;

    // JSON doesn't support comments
    // if (object->descriptionId.isDefined())
    // xml.writeComment(descriptions[object->descriptionId.value()]);

    /*
     Money is converted directly into copper-coins and set as a property on the parent object.
     */
    if (parent) {
        int quantity = 0;

        switch (object->prototype->id) {
        case 7000:
            if (object->quantity.isDefined())
                quantity = object->quantity.value();
            else
                quantity = 1;
            break;
        case 7001:
            if (object->quantity.isDefined())
                quantity = object->quantity.value() * 10;
            else
                quantity = 10;
            break;
        case 7002:
            if (object->quantity.isDefined())
                quantity = object->quantity.value() * 100;
            else
                quantity = 100;
            break;
        case 7003:
            if (object->quantity.isDefined())
                quantity = object->quantity.value() * 1000;
            else
                quantity = 1000;
            break;
        default:
            break;
        }

        if (quantity > 0) {
            bool ok;
            quantity += parent->value("money", 0).toInt(&ok);

            if (!ok) {
                qWarning("Parent has an invalid money entry.");
            }

            parent->insert("money", quantity);
            return QVariant(); // Don't actually convert the money objects
        }
    }

    if (!object->id.isNull())
        objectMap["id"] = object->id;

    objectMap["prototype"] = object->prototype->id;

    NonPlayerCharacterProperties *npcProperties = qobject_cast<NonPlayerCharacterProperties*>(object->prototype->additionalProperties);

    JsonPropertyWriter props(objectMap);
    if (object->name.isDefined()) {
        props.write("internalDescription", service->getInternalName(object->name.value()));
        props.write("internalId", object->name.value());
    }

    if (parent) {
        int inventoryLoc = 0;
        if (object->itemInventoryLocation.isDefined())
            inventoryLoc = object->itemInventoryLocation.value();
        if (inventoryLoc < 0)
            qWarning("Negative item inventory location. What does that mean? %d", inventoryLoc);
        if (inventoryLoc >= 200)
            props.write("slot", inventoryLoc);
    } else {
        QVariantList vector;
        vector.append(object->position.x());
        vector.append(object->position.y());
        vector.append(object->position.z());
        objectMap["position"] = QVariant(vector);
    }

    if (object->waypoints.isEmpty()) {
        object->flags.removeAll("WaypointsDay");
        object->flags.removeAll("WaypointsNight");
    }

    props.write("unknownDescriptionId", object->descriptionUnknownId);
    props.write("portraitId", object->portrait);
    props.write("flags", object->flags);
    props.write("scale", object->scale);
    props.write("rotation", object->rotation);
    props.write("radius", object->radius);
    props.write("height", object->renderHeight);
    props.write("sceneryFlags", object->sceneryFlags);
    props.write("descriptionId", object->descriptionId);
    props.write("secretDoorFlags", object->secretDoorFlags);
    props.write("portalFlags", object->portalFlags);
    props.write("lockDc", object->lockDc);
    props.write("teleportTarget", object->teleportTarget);
    // props.write("parentItemId", object->parentItemId);
    props.write("substituteInventoryId", object->substituteInventoryId);
    props.write("hitPoints", object->hitPoints);
    props.write("hitPointsAdjustment", object->hitPointsAdjustment);
    props.write("hitPointsDamage", object->hitPointsDamage);
    props.write("walkSpeedFactor", object->walkSpeedFactor);
    props.write("runSpeedFactor", object->runSpeedFactor);
    props.write("dispatcher", object->dispatcher);
    props.write("secretDoorEffect", object->secretDoorEffect);
    props.write("notifyNpc", object->notifyNpc);
    props.write("dontDraw", object->dontDraw);
    props.write("disabled", object->disabled);
    props.write("unlit", object->unlit);
    props.write("interactive", object->interactive);
    // props.write("containerFlags", object->containerFlags);
    props.write("containerInventoryId", object->containerInventoryId);
    props.write("containerInventoryListIndex", object->containerInventoryListIndex);
    props.write("containerInventorySource", object->containerInventorySource);
    props.write("itemFlags", object->itemFlags);
    props.write("weight", object->itemWeight);
    props.write("worth", object->itemWorth);
    props.write("quantity", object->quantity);
    props.write("weaponFlags", object->weaponFlags);
    props.write("armorFlags", object->armorFlags);
    props.write("armorAcAdjustment", object->armorAcAdjustment);
    props.write("armorMaxDexBonus", object->armorMaxDexBonus);
    props.write("armorCheckPenalty", object->armorCheckPenalty);
    props.write("keyId", object->keyId);
    if (object->critterFlags.removeAll("IsConcealed"))
        props.write("concealed", true);
    props.write("critterFlags", object->critterFlags);

    // They're used only on the tutorial map, thus we ignore them here.
    // props.write("critterFlags2", object->critterFlags2);
    // There are some mobs on temple level 1 that have this property, but it's always 0.
    // props.write("critterRace", object->critterRace);
    // Same as above, only this time its value is 1
    // props.write("gender", object->critterGender);
    props.write("critterMoneyIndex", object->critterMoneyIndex);
    props.write("critterInventoryNum", object->critterInventoryNum);
    props.write("critterInventorySource", object->critterInventorySource);
    if (object->npcFlags.removeAll("KillOnSight") && (!npcProperties || npcProperties->flags.contains("KillOnSight")))
        props.write("killsOnSight", true);
    props.write("npcFlags", object->npcFlags);
    props.write("factions", factionsToList(object->factions));
    props.write("locked", object->locked);

    props.write("strength", object->strength);
    props.write("dexterity", object->dexterity);
    props.write("constitution", object->constitution);
    props.write("intelligence", object->intelligence);
    props.write("wisdom", object->wisdom);
    props.write("charisma", object->charisma);

    // Standpoints
    props.write("standpointDay", standpointToMap(service, object->dayStandpoint));
    props.write("standpointNight", standpointToMap(service, object->nightStandpoint));
    props.write("standpointScout", standpointToMap(service, object->scoutStandpoint));

    if (!object->waypoints.isEmpty()) {
        QVariantList waypoints;

        foreach (const GameObject::Waypoint &waypoint, object->waypoints) {
            waypoints.append(waypointToMap(waypoint));
        }

        props.write("waypoints", waypoints);
    }

    if (!object->content.isEmpty()) {
        QVariantList content;
        foreach (GameObject *subObject, object->content) {
            QVariant convertedObj = toVariant(service, subObject, &objectMap);

            if (!convertedObj.isNull())
                content.append(convertedObj);
        }
        objectMap["content"] = content;
    }

    return objectMap;
}

static QVariantList convertLightingKeyframes(const QList<LightKeyframe> &keyframes)
{
    QVariantList result;

    foreach (const LightKeyframe &keyframe, keyframes) {
        QVariantList color;
        color.append(QVariant::fromValue<double>(keyframe.red));
        color.append(QVariant::fromValue<double>(keyframe.green));
        color.append(QVariant::fromValue<double>(keyframe.blue));

        QVariantList frameList;
        frameList.append(keyframe.hour);
        frameList.append(QVariant(color));

        result.append(QVariant(frameList));
    }

    return result;
}

void ConvertMapsTask::convertMapObject(ZoneTemplate *zoneTemplate, IFileWriter *writer)
{
    Serializer serializer;

    QVariantMap mapObject;

    mapObject["name"] = zoneTemplate->name() + " (" + QString::number(zoneTemplate->id()) + ")";

    //mapObject["pathNodes"] = convertPathNodes(zoneTemplate->directory(), service());

    mapObject["scrollBox"] = QVariantList() << zoneTemplate->scrollBox().minimum().x()
                             << zoneTemplate->scrollBox().minimum().y()
                             << zoneTemplate->scrollBox().maximum().x()
                             << zoneTemplate->scrollBox().maximum().y();

    QString areaId = getAreaFromMapId(zoneTemplate->id());
    if (!areaId.isEmpty())
        mapObject["area"] = areaId;

    if (zoneTemplate->dayBackground()) {
        mapObject["dayBackground"] = getNewBackgroundMapFolder(zoneTemplate->dayBackground()->directory());
    }
    if (zoneTemplate->nightBackground()) {
        mapObject["nightBackground"] = getNewBackgroundMapFolder(zoneTemplate->nightBackground()->directory());
    }
    QVector3D startPos(zoneTemplate->startPosition().x(), 0, zoneTemplate->startPosition().y());
    mapObject["startPosition"] = vectorToList(startPos);
    if (zoneTemplate->movie())
        mapObject["movie"] = zoneTemplate->movie();
    mapObject["outdoor"] = zoneTemplate->isOutdoor();
    mapObject["unfogged"] = zoneTemplate->isUnfogged();
    // This is simply not used by us anymore
    // mapObject["dayNightTransfer"] = zoneTemplate->hasDayNightTransfer();
    // The scripts ignore this flag anyway. We should insert map-specific code instead.
    // mapObject["allowsBedrest"] = zoneTemplate->allowsBedrest();
    mapObject["menuMap"] = zoneTemplate->isMenuMap();
    mapObject["tutorialMap"] = zoneTemplate->isTutorialMap();
    mapObject["clippingGeometry"] = zoneTemplate->directory() + "clipping.dat";
    mapObject["regions"] = zoneTemplate->directory() + "regions.dat";

    QVariantList soundSchemes;
    foreach (uint schemeId, zoneTemplate->soundSchemes()) {
        soundSchemes.append(QString("scheme-%1").arg(schemeId));
    }
    mapObject["soundSchemes"] = soundSchemes;

    QVariantMap globalLightMap;

    if (!zoneTemplate->lightingKeyframesDay2d().isEmpty()
        && !zoneTemplate->lightingKeyframesDay3d().isEmpty()
        && !zoneTemplate->lightingKeyframesNight2d().isEmpty()
        && !zoneTemplate->lightingKeyframesNight3d().isEmpty()) {

        globalLightMap["day2dKeyframes"] = convertLightingKeyframes(zoneTemplate->lightingKeyframesDay2d());
        globalLightMap["night2dKeyframes"] = convertLightingKeyframes(zoneTemplate->lightingKeyframesNight2d());
        globalLightMap["day3dKeyframes"] = convertLightingKeyframes(zoneTemplate->lightingKeyframesDay3d());
        globalLightMap["night3dKeyframes"] = convertLightingKeyframes(zoneTemplate->lightingKeyframesNight3d());
    }

    Light globalLight = zoneTemplate->globalLight();
    globalLightMap["color"] = QVariantList() << globalLight.r / 255.0 << globalLight.g / 255.0 << globalLight.b / 255.0;
    globalLightMap["direction"] = QVariantList() << globalLight.dirX << globalLight.dirY << globalLight.dirZ;

    mapObject["globalLighting"] = globalLightMap;

    QVariantList lightList;
    foreach (const Light &light, zoneTemplate->lights()) {
        QVariantMap lightMap;

        lightMap["day"] = light.day;
        lightMap["type"] = light.type; // 1 = Point, 2 = Spot, 3 = Directional
        lightMap["color"] = QVariantList() << light.r / 255.0 << light.g / 255.0 << light.b / 255.0 << 0;
        lightMap["position"] = QVariantList() << light.position.x() << light.position.y() << light.position.z();
        // 1 == PointLight
        if (light.type != 1)
            lightMap["direction"] = QVariantList() << light.dirX << light.dirY << light.dirZ;
        if (light.handle)
            lightMap["handle"] = light.handle;
        lightMap["range"] = light.range;
        // 2 == SpotLight
        if (light.type == 2)
            lightMap["phi"] = light.phi;

        lightList.append(lightMap);
    }
    if (!lightList.isEmpty()) {
        writer->addFile(zoneTemplate->directory() + "lights.js", serializer.serialize(lightList));
        mapObject["lights"] = zoneTemplate->directory() + "lights.js";
    }

    QVariantList particleSystemList;

    foreach (const ParticleSystem &particleSystem, zoneTemplate->particleSystems()) {
        QVariantMap particleSystemMap;

        QString name = service()->getParticleSystemFromHash(particleSystem.hash);

        if (name.isNull())
            continue;

        particleSystemMap["day"] = particleSystem.light.day;
        particleSystemMap["name"] = name;
        particleSystemMap["id"] = particleSystem.id;
        Light light = particleSystem.light;
        particleSystemMap["position"] = QVariantList() << light.position.x() << light.position.y() << light.position.z();

        particleSystemList.append(particleSystemMap);
    }
    if (!particleSystemList.isEmpty()) {
        writer->addFile(zoneTemplate->directory() + "particleSystems.js", serializer.serialize(particleSystemList));
        mapObject["particleSystems"] = zoneTemplate->directory() + "particleSystems.js";
    }

    // Convert dynamic objects. We'll add some object to this list from the static objects.
    QVariantList dynamicObjects;

    QVariantList objectList;
    foreach (GameObject *object, zoneTemplate->staticObjects()) {
        bool moveToDynamics = false;

        /*
         This is a special hack that assigns GUIDs to portals so state for them can be persisted. They're pretty much
         the only type of *static* object in a map that can be interacted with by the player (except for level-
         transition objects maybe).
         */
        if (object->prototype->type == Portal) {
            moveToDynamics = true;
        }
        /*
          This is a rather obnoxious property of the ToEE scripts. There are static objects, that
          are actually referenced and manipulated by the ToEE scripts. One example for this is the
          minotaur statue on the 2nd temple level. It's deleted, but it's a static object too!

          Solution: Move it over to the dynamic objects and assign it a GUID.
          */
        if (object->prototype->internalDescriptionId.isDefined()) {
            moveToDynamics = true;
        }

        if (moveToDynamics) {
            if (object->id.isEmpty())
                object->id = QUuid::createUuid().toString();

            dynamicObjects.append(toVariant(service(), object));
            continue;
        }

        objectList.append(toVariant(service(), object));
    }

    // Now add dynamic objects here
    foreach (GameObject *object, zoneTemplate->mobiles()) {
        dynamicObjects.append(toVariant(service(), object));
    }

    // Static geometry objects are treated the same way, although they don't have
    // a prototype.
    foreach (GeometryObject *object, zoneTemplate->staticGeometry()) {
        QVariantMap map;

        map["prototype"] = "StaticGeometry";
        map["model"] = getNewModelFilename(object->mesh());
        map["position"] = vectorToList(object->position());
        if (object->rotation() != 0)
            map["rotation"] = object->rotation();
        float scale = object->scale().x();
        if (scale != 1)
            map["scale"] = object->scale().x() * 100;
        // The assumption here is, that for static objects the scale is uniform
        Q_ASSERT_X(scale == object->scale().y() && scale == object->scale().z(), "convertStaticObjects",
                 qPrintable(QString("%1 %2 %3").arg(scale).arg(object->scale().y()).arg(object->scale().z())));

        objectList.append(map);
    }

    if (!objectList.isEmpty())
        mapObject["staticObjects"] = zoneTemplate->directory() + "staticObjects.js";
    if (!dynamicObjects.isEmpty())
    mapObject["mobileObjects"] = zoneTemplate->directory() + "mobiles.js";

    writer->addFile(zoneTemplate->directory() + "map.js", serializer.serialize(mapObject));

    writer->addFile(zoneTemplate->directory() + "staticObjects.js", serializer.serialize(objectList));

    writer->addFile(zoneTemplate->directory() + "mobiles.js", serializer.serialize(dynamicObjects));
}

void convertClippingMeshes(IConversionService *service, ZoneTemplate *zoneTemplate, IFileWriter *writer)
{
    ClippingMeshConverter converter(service, zoneTemplate);
    QByteArray clippingData = converter.convert();

    writer->addFile(zoneTemplate->directory() + "clipping.dat", clippingData);
}

ConvertMapsTask::ConvertMapsTask(IConversionService *service, QObject *parent) : ConversionTask(service, parent)
{
    if (!mMapExclusions.load(":/map_exclusions.txt")) {
        qWarning("Unable to load map exclusions.");
    }
}

uint ConvertMapsTask::cost() const
{
    return 50;
}

QString ConvertMapsTask::description() const
{
    return "Converting maps";
}

static QHash< uint, QList<QPoint> > loadJumpPoints(Troika::VirtualFileSystem *vfs)
{
    QHash< uint, QList<QPoint> > result;

    QString jumpPoints = QString::fromLocal8Bit(vfs->openFile("rules/jumppoint.tab"));

    QStringList lines = jumpPoints.split("\n");

    foreach (const QString &line, lines) {
        QStringList parts = line.split("\t");
        if (parts.size() != 5)
            continue;

        uint map = parts[2].toUInt();
        uint x = parts[3].toUInt();
        uint y = parts[4].toUInt();

        if (map && x && y) {
            QPoint p;
            p.setX((x + 0.5) * PixelPerWorldTile);
            p.setY((y + 0.5) * PixelPerWorldTile);

            QList<QPoint> points = result[map];
            points.append(p);
            result[map] = points;
        }
    }

    return result;
}

void ConvertMapsTask::run()
{
    QScopedPointer<IFileWriter> backgroundOutput(service()->createOutput("backgrounds"));
    QScopedPointer<IFileWriter> mapsOutput(service()->createOutput("maps"));
    MapConverter converter(service(), backgroundOutput.data());

    ZoneTemplates *zoneTemplates = service()->zoneTemplates();

    int totalWork = zoneTemplates->mapIds().size();
    int workDone = 0;

    // Convert all maps
    foreach (quint32 mapId, zoneTemplates->mapIds()) {
        assertNotAborted();
        ++workDone;

        if (mMapExclusions.isExcluded(QString("%1").arg(mapId)))
            continue;

        QScopedPointer<Troika::ZoneTemplate> zoneTemplate(zoneTemplates->load(mapId));

        if (zoneTemplate) {
            Troika::ZoneBackgroundMap *background = zoneTemplate->dayBackground();

            if (background) {
                qDebug("Converting map %d. Name: %s. Dir: %s", zoneTemplate->id(),
                       qPrintable(zoneTemplate->name()), qPrintable(background->directory()));
            } else {
                qWarning("Zone has no daylight background: %d (SKIPPING).", mapId);
                continue;
            }

            QElapsedTimer timer;
            timer.start();

            converter.convert(zoneTemplate.data());

            qDebug("Converted background map in %d ms.", timer.restart());

            // Add all static geometry files to the list of referenced models
            foreach (Troika::GeometryObject *object, zoneTemplate->staticGeometry()) {
                service()->addMeshReference(object->mesh());
            }

            convertMapObject(zoneTemplate.data(), mapsOutput.data());

            qDebug("Converted map objects in %d ms.", timer.restart());

            convertClippingMeshes(service(), zoneTemplate.data(), mapsOutput.data());

            qDebug("Converted clipping meshes in %d ms.", timer.restart());

            SectorConverter converter(zoneTemplate.data());
            mapsOutput->addFile(zoneTemplate->directory() + "tileinfo.dat", converter.convert());

            qDebug("Converted tile information in %d ms.", timer.restart());
        } else {
            qWarning("Unable to load zone template for map id %d.", mapId);
        }

        emit progress(workDone, totalWork);
    }

    backgroundOutput->close();
    mapsOutput->close();
}

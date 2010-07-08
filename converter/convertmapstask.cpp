
#include "qbox3d.h"

#include "serializer.h"
using namespace QJson;

#include "convertmapstask.h"
#include "mapconverter.h"
#include "zonetemplates.h"
#include "zonebackgroundmap.h"
#include "messagefile.h"
#include "pathnodeconverter.h"
#include "util.h"
#include "dagreader.h"
#include "virtualfilesystem.h"
#include "navigationmeshbuilder.h"
#include "model.h"

static QVariantMap convertPathNodes(const QString &directory, IConversionService *service)
{
    PathNodeConverter converter;

    QString filename = directory + "pathnode.pnd";

    if (!converter.load(service->virtualFileSystem()->openFile(filename))) {
        qWarning("Unable to load pathnode data %s: %s.", qPrintable(filename), qPrintable(converter.error()));
        return QVariantMap();
    }

    return converter.convert();
}

static QVariant toVariant(IConversionService *service, GameObject *object)
{
    QVariantMap objectMap;

    // JSON doesn't support comments
    // if (object->descriptionId.isDefined())
    // xml.writeComment(descriptions[object->descriptionId.value()]);

    if (!object->id.isNull())
        objectMap["id"] = object->id;

    objectMap["prototype"] = object->prototype->id;

    QVariantList vector;
    vector.append(object->position.x());
    vector.append(object->position.y());
    vector.append(object->position.z());
    objectMap["position"] = QVariant(vector);

    JsonPropertyWriter props(objectMap);
    if (object->name.isDefined())
        props.write("internalDescription", service->getInternalName(object->name.value()));
    props.write("flags", object->flags);
    props.write("scale", object->scale);
    props.write("rotation", object->rotation);
    props.write("radius", object->radius);
    props.write("height", object->renderHeight);
    props.write("sceneryFlags", object->sceneryFlags);
    props.write("descriptionId", object->descriptionId);
    props.write("secretDoorFlags", object->secretDoorFlags);
    props.write("portalFlags", object->portalFlags);
    props.write("portalLockDc", object->portalLockDc);
    props.write("teleportTarget", object->teleportTarget);
    // props.write("parentItemId", object->parentItemId);
    props.write("substituteInventoryId", object->substituteInventoryId);
    props.write("itemInventoryLocation", object->itemInventoryLocation);
    props.write("hitPoints", object->hitPoints);
    props.write("hitPointsAdjustment", object->hitPointsAdjustment);
    props.write("hitPointsDamage", object->hitPointsDamage);
    props.write("walkSpeedFactor", object->walkSpeedFactor);
    props.write("runSpeedFactor", object->runSpeedFactor);
    props.write("dispatcher", object->dispatcher);
    props.write("secretDoorEffect", object->secretDoorEffect);
    props.write("notifyNpc", object->notifyNpc);
    props.write("containerFlags", object->containerFlags);
    props.write("containerLockDc", object->containerLockDc);
    props.write("containerKeyId", object->containerKeyId);
    props.write("containerInventoryId", object->containerInventoryId);
    props.write("containerInventoryListIndex", object->containerInventoryListIndex);
    props.write("containerInventorySource", object->containerInventorySource);
    props.write("itemFlags", object->itemFlags);
    props.write("itemWeight", object->itemWeight);
    props.write("itemWorth", object->itemWorth);
    props.write("itemQuantity", object->itemQuantity);
    props.write("weaponFlags", object->weaponFlags);
    props.write("ammoQuantity", object->ammoQuantity);
    props.write("armorFlags", object->armorFlags);
    props.write("armorAcAdjustment", object->armorAcAdjustment);
    props.write("armorMaxDexBonus", object->armorMaxDexBonus);
    props.write("armorCheckPenalty", object->armorCheckPenalty);
    props.write("moneyQuantity", object->moneyQuantity);
    props.write("keyId", object->keyId);
    props.write("critterFlags", object->critterFlags);
    props.write("critterFlags2", object->critterFlags2);
    props.write("critterRace", object->critterRace);
    props.write("critterGender", object->critterGender);
    props.write("critterMoneyIndex", object->critterMoneyIndex);
    props.write("critterInventoryNum", object->critterInventoryNum);
    props.write("critterInventorySource", object->critterInventorySource);
    props.write("npcFlags", object->npcFlags);

    if (!object->content.isEmpty()) {
        QVariantList content;
        foreach (GameObject *subObject, object->content) {
            content.append(toVariant(service, subObject));
        }
        objectMap["content"] = content;
    }

    return objectMap;
}

void ConvertMapsTask::convertStaticObjects(ZoneTemplate *zoneTemplate, IFileWriter *writer)
{
    QHash<uint,QString> descriptions = MessageFile::parse(service()->virtualFileSystem()->openFile("mes/description.mes"));

    QVariantMap mapObject;

    mapObject["name"] = zoneTemplate->name();

    mapObject["waypoints"] = convertPathNodes(zoneTemplate->directory(), service());

    mapObject["scrollBox"] = QVariantList() << zoneTemplate->scrollBox().minimum().x()
                             << zoneTemplate->scrollBox().minimum().y()
                             << zoneTemplate->scrollBox().maximum().x()
                             << zoneTemplate->scrollBox().maximum().y();

    mapObject["id"] = zoneTemplate->id(); // This is actually legacy...
    if (zoneTemplate->dayBackground()) {
        mapObject["dayBackground"] = getNewBackgroundMapFolder(zoneTemplate->dayBackground()->directory());
    }
    if (zoneTemplate->nightBackground()) {
        mapObject["nightBackground"] = getNewBackgroundMapFolder(zoneTemplate->dayBackground()->directory());
    }
    QVector3D startPos(zoneTemplate->startPosition().x(), 0, zoneTemplate->startPosition().y());
    mapObject["startPosition"] = vectorToList(startPos);
    if (zoneTemplate->movie())
        mapObject["movie"] = zoneTemplate->movie();
    mapObject["outdoor"] = zoneTemplate->isOutdoor();
    mapObject["unfogged"] = zoneTemplate->isUnfogged();
    mapObject["dayNightTransfer"] = zoneTemplate->hasDayNightTransfer();
    mapObject["allowsBedrest"] = zoneTemplate->allowsBedrest();
    mapObject["menuMap"] = zoneTemplate->isMenuMap();
    mapObject["tutorialMap"] = zoneTemplate->isTutorialMap();
    mapObject["clippingGeometry"] = zoneTemplate->directory() + "clipping.dat";

    Light globalLight = zoneTemplate->globalLight();
    QVariantMap globalLightMap;
    globalLightMap["color"] = QVariantList() << globalLight.r / 255.0 << globalLight.g / 255.0 << globalLight.b / 255.0 << 0;
    globalLightMap["direction"] = QVariantList() << globalLight.dirX << globalLight.dirY << globalLight.dirZ;
    mapObject["globalLight"] = globalLightMap;

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
    mapObject["lights"] = lightList;

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

    mapObject["particleSystems"] = particleSystemList;

    QVariantList objectList;
    foreach (GameObject *object, zoneTemplate->staticObjects()) {
        objectList.append(toVariant(service(), object));
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

    mapObject["staticObjects"] = objectList;

    objectList.clear();

    foreach (GameObject *object, zoneTemplate->mobiles()) {
        objectList.append(toVariant(service(), object));
    }
    mapObject["dynamicObjects"] = objectList;

    Serializer serializer;
    QByteArray data = serializer.serialize(mapObject);

    writer->addFile(zoneTemplate->directory() + "map.js", data, 9);
}

bool compareScale(const Vector4 &a, const Vector4 &b) {
    float scaleEpsilon = 0.0001f;

    Vector4 diff = (b - a).absolute();

    return (diff.x() <= scaleEpsilon && diff.y() <= scaleEpsilon && diff.z() <= scaleEpsilon);
}

void convertClippingMeshes(IConversionService *service, ZoneTemplate *zoneTemplate, IFileWriter *writer)
{
    QByteArray clippingData;
    QDataStream clippingStream(&clippingData, QIODevice::WriteOnly);
    clippingStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    clippingStream.setByteOrder(QDataStream::LittleEndian);

    // Create a list of all clipping geometry meshes
    QStringList clippingGeometryFiles;
    QList< QList<Vector4> > scalingPerFile;
    foreach (GeometryObject *object, zoneTemplate->clippingGeometry()) {
        QString normalizedFilename = QDir::toNativeSeparators(object->mesh()).toLower();

        if (!clippingGeometryFiles.contains(normalizedFilename)) {
            clippingGeometryFiles.append(normalizedFilename);
            scalingPerFile.append(QList<Vector4>());
        }
    }

    /**
    For each geometry file we will collect all the scales in which it is used.
    This is important since ToEE uses non-uniform scaling here (it scales in 2d space),
    and we want to remove that scaling entirely. Instead, we will create a copy of the geometry
    for each scaling level used.
      */
    foreach (GeometryObject *object, zoneTemplate->clippingGeometry()) {
        QString normalizedFilename = QDir::toNativeSeparators(object->mesh()).toLower();
        int fileIndex = (uint)clippingGeometryFiles.indexOf(normalizedFilename);

        Vector4 scale(object->scale().x(), object->scale().y(), object->scale().z(), 1);

        bool scaleExists = false;
        foreach (Vector4 existingScale, scalingPerFile[fileIndex]) {
            if (compareScale(existingScale, scale)) {
                scaleExists = true;
                break;
            }
        }

        if (!scaleExists)
            scalingPerFile[fileIndex].append(scale);
    }

    uint totalFileCount = 0;

    // Now we test if for any geometry object there are more than one scale versions
    for (int i = 0; i < clippingGeometryFiles.size(); ++i) {
        totalFileCount += scalingPerFile[i].size();
#ifndef QT_NO_DEBUG
        if (scalingPerFile[i].size() > 1) {
            qDebug("Clipping geometry file %s is used in %d different scales:", qPrintable(clippingGeometryFiles[i]), scalingPerFile[i].size());
        }
#endif
    }

    // File header
    clippingStream << totalFileCount << (uint)zoneTemplate->clippingGeometry().size();

    /**
    These transformations come from the original game and are *constant*.
    **/
    // Old: -44
    Quaternion rot1 = Quaternion::fromAxisAndAngle(1, 0, 0, -0.77539754f);
    Matrix4 rotate1matrix = Matrix4::transformation(Vector4(1,1,1,0), rot1, Vector4(0,0,0,0));

    // Old: 90-135
    Quaternion rot2 = Quaternion::fromAxisAndAngle(0, 1, 0, 2.3561945f);
    Matrix4 rotate2matrix = Matrix4::transformation(Vector4(1,1,1,0), rot2, Vector4(0,0,0,0));

    Matrix4 baseView = rotate1matrix * rotate2matrix;
    Matrix4 baseViewInverse = baseView.inverted();

    for (int i = 0; i < clippingGeometryFiles.size(); ++i) {
        Troika::DagReader reader(service->virtualFileSystem(), clippingGeometryFiles[i]);
        MeshModel *model = reader.get();

        Q_ASSERT(model->faceGroups().size() == 1);
        Q_ASSERT(model->vertices().size() > 0);

        /**
          Calculate the scaling matrix for each instance and transform the vertices ahead-of-time.
          */
        foreach (Vector4 scale, scalingPerFile[i]) {
            QVector<Vector4> scaledVertices(model->vertices().size());

            // Switch z/y
            if (qFuzzyCompare(scale.x(), 1) && qFuzzyCompare(scale.y(), 1) && qFuzzyCompare(scale.z(), 1)) {
                for (int j = 0; j < scaledVertices.size(); ++j) {
                    const Troika::Vertex &vertex = model->vertices()[j];
                    Vector4 pos(vertex.positionX, vertex.positionY, vertex.positionZ, 1);
                    scaledVertices[j] = pos;
                }
            } else {
                Matrix4 scaleMatrix2d = Matrix4::scaling(scale.x(), scale.z(), scale.y());
                Matrix4 scaleMatrix = baseViewInverse * scaleMatrix2d * baseView;

                for (int j = 0; j < scaledVertices.size(); ++j) {
                    const Troika::Vertex &vertex = model->vertices()[j];
                    Vector4 pos(vertex.positionX, vertex.positionY, vertex.positionZ, 1);
                    scaledVertices[j] = scaleMatrix.mapPosition(pos);
                }
            }

            Box3d boundingBox(scaledVertices[0], scaledVertices[0]);

            Vector4 firstVertex = scaledVertices[0];
            firstVertex.setW(0);
            float originDistanceSquared = firstVertex.lengthSquared();

            for (int j = 1; j < scaledVertices.size(); ++j) {
                Vector4 vertex = scaledVertices[j];
                boundingBox.merge(vertex);
                vertex.setW(0);
                originDistanceSquared = qMax<float>(originDistanceSquared, vertex.lengthSquared());
            }

            float originDistance = sqrt(originDistanceSquared);

            clippingStream << boundingBox.minimum().x() << boundingBox.minimum().y() << boundingBox.minimum().z()
                    << boundingBox.minimum().w() << boundingBox.maximum().x() << boundingBox.maximum().y()
                    << boundingBox.maximum().z() << boundingBox.maximum().w() << originDistance << originDistanceSquared;

            clippingStream << (uint)scaledVertices.size() << (uint)model->faceGroups()[0]->faces().size() * 3;

            // We should apply the scaling here.

            foreach (const Vector4 &vertex, scaledVertices) {
                clippingStream << vertex.x() << vertex.y() << vertex.z() << vertex.w();
            }

            foreach (const Face &face, model->faceGroups()[0]->faces()) {
                clippingStream << face.vertices[0] << face.vertices[1] << face.vertices[2];
            }
        }
    }

    // Instances
    foreach (GeometryObject *object, zoneTemplate->clippingGeometry()) {
        QString normalizedFilename = QDir::toNativeSeparators(object->mesh()).toLower();

        uint fileNameIndex = (uint)clippingGeometryFiles.indexOf(normalizedFilename);
        uint fileIndex = 0;

        // Find the filename/scale pair appropriate for this instance
        for (uint i = 0; i < fileNameIndex; ++i)
            fileIndex += scalingPerFile[i].size();

        bool found = false;

        Vector4 scale(object->scale().x(), object->scale().y(), object->scale().z(), 1);
        for (int i = 0; i < scalingPerFile[fileNameIndex].size(); ++i) {
            Vector4 existingScale = scalingPerFile[fileNameIndex][i];
            if (compareScale(existingScale, scale)) {
                found = true;
                break;
            }
            fileIndex++;
        }

        Q_ASSERT(found);

        clippingStream << object->position().x() << object->position().y() << object->position().z() << (float)1
                << deg2rad(object->rotation()) << (float)1
                << fileIndex;
    }

    writer->addFile(zoneTemplate->directory() + "clipping.dat", clippingData, 9);
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

void ConvertMapsTask::run()
{
    QScopedPointer<IFileWriter> writer(service()->createOutput("maps"));
    MapConverter converter(service(), writer.data());

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

        converter.convert(zoneTemplate.data());

        if (zoneTemplate) {
            Troika::ZoneBackgroundMap *background = zoneTemplate->dayBackground();

            if (background) {
                qDebug("Loaded map %d. Name: %s. Dir: %s", zoneTemplate->id(),
                       qPrintable(zoneTemplate->name()), qPrintable(background->directory()));
            } else {
                qWarning("Zone has no daylight background: %d.", mapId);
            }

            // Add all static geometry files to the list of referenced models
            foreach (Troika::GeometryObject *object, zoneTemplate->staticGeometry()) {
                service()->addMeshReference(object->mesh());
            }

            convertStaticObjects(zoneTemplate.data(), writer.data());

            convertClippingMeshes(service(), zoneTemplate.data(), writer.data());

            QVector<QPoint> startPositions;
            startPositions.append(zoneTemplate->startPosition());
            QByteArray navmeshes = NavigationMeshBuilder::build(zoneTemplate.data(), startPositions);
            writer->addFile(zoneTemplate->directory() + "regions.dat", navmeshes);
        } else {
            qWarning("Unable to load zone template for map id %d.", mapId);
        }

        emit progress(workDone, totalWork);
    }
}
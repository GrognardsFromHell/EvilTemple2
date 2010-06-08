
#include <QtGlobal>
#include <QtXml>
#include <QDataStream>
#include <QScopedPointer>
#include <QFileInfo>
#include <QTextStream>
#include <QBuffer>
#include <QScriptEngine>

#include <iostream>

#include "dagreader.h"
#include "zipwriter.h"
#include "virtualfilesystem.h"
#include "messagefile.h"
#include "troikaarchive.h"
#include "prototypes.h"
#include "troika_materials.h"
#include "zonetemplate.h"
#include "zonetemplatereader.h"
#include "zonetemplates.h"
#include "zonebackgroundmap.h"
#include "skmreader.h"
#include "model.h"
#include "material.h"
#include "exclusions.h"
#include "mapconverter.h"
#include "prototypeconverter.h"

#include "util.h"
#include "converter.h"
#include "materialconverter.h"
#include "interfaceconverter.h"
#include "modelwriter.h"

#include "serializer.h"
using namespace QJson;

void printEventTypes();

/**
  The original game applies an additional base rotation to everything in order to align it
  with the isometric grid. This is the radians value of that rotation.
  */
const float LegacyBaseRotation = 0.77539754f;

struct MeshReference {
    MeshReference() : meshId(0), source(0) {}

    enum Sources {
        MeshesMes = 0x1,
        StaticGeometry = 0x2, // from a map
        AddMesh = 0x4, // Added from addmesh.mes
        Hair = 0x8, // Hair
        FORCE_DWORD = 0x7FFFFFFF, // force 32-bit size (whatever)
    };

    uint meshId; // From meshes.mes
    uint source; // bitfield
};

static QScriptValue readMesFile(QScriptContext *ctx, QScriptEngine *engine, Troika::VirtualFileSystem *vfs) {
    if (ctx->argumentCount() != 1) {
        return ctx->throwError("readMesFile takes one string argument.");
    }

    QString filename = ctx->argument(0).toString();

    QHash<uint,QString> mapping = MessageFile::parse(vfs->openFile(filename));

    QScriptValue result = engine->newObject();

    foreach (uint key, mapping.keys()) {
        result.setProperty(QString("%1").arg(key), QScriptValue(mapping[key]));
    }

    return result;
}

static QScriptValue addFile(QScriptContext *ctx, QScriptEngine *engine, ZipWriter *writer) {
    if (ctx->argumentCount() != 3) {
        return ctx->throwError("addFile takes three arguments: filename, content, compression.");
    }

    QString filename = ctx->argument(0).toString();
    QString content = ctx->argument(1).toString();
    int compression = ctx->argument(2).toInt32();

    writer->addFile(filename, content.toUtf8(), compression);

    return QScriptValue(true);
}

static QScriptValue readTabFile(QScriptContext *ctx, QScriptEngine *engine, Troika::VirtualFileSystem *vfs) {
    if (ctx->argumentCount() != 1) {
        return ctx->throwError("readTabFile takes one string argument.");
    }

    QString filename = ctx->argument(0).toString();

    QByteArray content = vfs->openFile(filename);
    QList<QByteArray> lines = content.split('\n');
    QList< QList<QByteArray> > tabFileContent;

    for (int i = 0; i < lines.length(); ++i) {
        QByteArray line = lines[i];
        if (line.endsWith('\r')) {
            line = line.left(line.length() - 1);
        }

        if (line.isEmpty())
            continue;

        tabFileContent.append(line.split('\t'));
    }

    QScriptValue result = engine->newArray(tabFileContent.length());

    for (int i = 0; i < tabFileContent.length(); ++i) {
        QList<QByteArray> line = tabFileContent[i];
        QScriptValue record = engine->newArray(line.length());
        for (int j = 0; j < line.length(); ++j) {
            record.setProperty(j, QScriptValue(QString(line[j])));
        }
        result.setProperty(i, record);
    }

    return result;
}

class ConverterData {
public:
    static const int maxArchives = 10; // Try ToEE0.dat to ToEE(maxArchives-1).dat

    QString mInputPath, mOutputPath;

    QScopedPointer<Troika::VirtualFileSystem> vfs;

    QScopedPointer<Troika::Prototypes> prototypes;

    QScopedPointer<Troika::ZoneTemplates> zoneTemplates;

    QScopedPointer<Troika::Materials> materials;

    QScopedPointer<ZipWriter> commonFiles;

    QHash<uint,QString> particleSystemHashes;

    QHash<QString,bool> mWrittenTextures;

    QHash<QString,bool> mWrittenMaterials;

    QHash<uint, QString> mInternalDescription;

    bool external;

    volatile bool cancel;

    // Maps lower-case mesh filenames (normalized separators) to an information structure
    QHash<QString, MeshReference> meshReferences;

    Exclusions exclusions;

    Converter *converter;

    int sectionsDone;
    static const int totalWorkSections = 100;
    int lastProgressUpdate;

    ConverterData(Converter *_converter, const QString &inputPath, const QString &outputPath) :
            converter(_converter), mInputPath(inputPath), mOutputPath(outputPath), external(false), cancel(false),
            sectionsDone(0), lastProgressUpdate(0)
    {
        // Force both paths to be in the system specific format and end with a separator.
        mInputPath = QDir::toNativeSeparators(mInputPath);
        if (!mInputPath.endsWith(QDir::separator()))
            mInputPath.append(QDir::separator());

        mOutputPath = QDir::toNativeSeparators(mOutputPath);
        if (!mOutputPath.endsWith(QDir::separator()))
            mOutputPath.append(QDir::separator());
        
        exclusions.load();
    }

    void loadArchives() {
        // Add base archives
        for (int i = 0; i < maxArchives; ++i) {
            QString archivePath = QString("%1ToEE%2.dat").arg(mInputPath).arg(i);

            if (QFile::exists(archivePath)) {
                qDebug("Adding archive %s", qPrintable(archivePath));
                vfs->add(new Troika::TroikaArchive(archivePath, vfs.data()));
            }
        }

        QString modulePath = QString("%1modules%2ToEE.dat").arg(mInputPath).arg(QDir::separator());
        qDebug("Adding archive %s", qPrintable(modulePath));
        vfs->add(new Troika::TroikaArchive(modulePath, vfs.data()));
    }

    bool convertMaps()
    {
        ZipWriter writer(mOutputPath + "maps.zip");
        MapConverter converter(vfs.data(), &writer);

        int totalWork = zoneTemplates->mapIds().size();
        int workDone = 0;

        // Convert all maps
        foreach (quint32 mapId, zoneTemplates->mapIds()) {
            if (cancel)
                return false;

            ++workDone;

            if (exclusions.isExcluded(QString("%1").arg(mapId)))
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
                    MeshReference &reference = meshReferences[QDir::toNativeSeparators(object->mesh()).toLower()];
                    reference.source |= MeshReference::StaticGeometry;
                }

                convertStaticObjects(zoneTemplate.data(), &writer);

                convertClippingMeshes(zoneTemplate.data(), &writer);
            } else {
                qWarning("Unable to load zone template for map id %d.", mapId);
            }

            updateProgress(workDone, totalWork, 25, "Converting maps");
        }

        writer.close();

        return true;
    }

    QVariantList vectorToList(QVector3D vector) {
        QVariantList result;
        result.append(vector.x());
        result.append(vector.y());
        result.append(vector.z());
        return result;
    }

    QVariant toVariant(GameObject *object)
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
            props.write("internalDescription", mInternalDescription[object->name.value()]);
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
                content.append(toVariant(subObject));
            }
            objectMap["content"] = content;
        }

        return objectMap;
    }

    void convertStaticObjects(ZoneTemplate *zoneTemplate, ZipWriter *writer)
    {
        QHash<uint,QString> descriptions = MessageFile::parse(vfs->openFile("mes/description.mes"));

        QVariantMap mapObject;

        mapObject["name"] = zoneTemplate->name();

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
        mapObject["startPosition"] = vectorToList(zoneTemplate->startPosition());
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
        globalLightMap["color"] = QVariantList() << globalLight.r << globalLight.g << globalLight.b;
        globalLightMap["direction"] = QVariantList() << globalLight.dirX << globalLight.dirY << globalLight.dirZ;
        mapObject["globalLight"] = globalLightMap;

        QVariantList lightList;
        foreach (const Light &light, zoneTemplate->lights()) {
            QVariantMap lightMap;

            lightMap["day"] = light.day;
            lightMap["type"] = light.type; // 1 = Point, 2 = Spot, 3 = Directional
            lightMap["color"] = QVariantList() << light.r << light.g << light.b;
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

            // Look up the hash of the particle system
            if (!particleSystemHashes.contains(particleSystem.hash))
                continue;

            particleSystemMap["day"] = particleSystem.light.day;
            particleSystemMap["name"] = particleSystemHashes[particleSystem.hash];
            particleSystemMap["id"] = particleSystem.id;
            Light light = particleSystem.light;
            particleSystemMap["position"] = QVariantList() << light.position.x() << light.position.y() << light.position.z();

            particleSystemList.append(particleSystemMap);
        }

        mapObject["particleSystems"] = particleSystemList;

        QVariantList objectList;
        foreach (GameObject *object, zoneTemplate->staticObjects()) {
            objectList.append(toVariant(object));
        }

        // Static geometry objects are treated the same way, although they don't have
        // a prototype.
        foreach (GeometryObject *object, zoneTemplate->staticGeometry()) {
            QVariantMap map;

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
            objectList.append(toVariant(object));
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

    void convertClippingMeshes(ZoneTemplate *zoneTemplate, ZipWriter *writer)
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
            if (scalingPerFile[i].size() > 1) {
                qDebug("Clipping geometry file %s is used in %d different scales:", qPrintable(clippingGeometryFiles[i]), scalingPerFile[i].size());
            }
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
            Troika::DagReader reader(vfs.data(), clippingGeometryFiles[i]);
            MeshModel *model = reader.get();

            Q_ASSERT(model->faceGroups().size() == 1);
            Q_ASSERT(model->vertices().size() > 0);

            /**
              Calculate the scaling matrix for each instance and transform the vertices ahead-of-time.
              */
            foreach (Vector4 scale, scalingPerFile[i]) {
                QVector<Vector4> scaledVertices(model->vertices().size());
                
                // Switch z/y
                Matrix4 scaleMatrix2d = Matrix4::scaling(scale.x(), scale.z(), scale.y());
                Matrix4 scaleMatrix = baseViewInverse * scaleMatrix2d * baseView;
                
                for (int j = 0; j < scaledVertices.size(); ++j) {
                    const Troika::Vertex &vertex = model->vertices()[j];
                    scaledVertices[j] = scaleMatrix.mapPosition(Vector4(vertex.positionX, vertex.positionY, vertex.positionZ, 1));
                }

                Box3d boundingBox(scaledVertices[0], scaledVertices[0]);

                Vector4 firstVertex = scaledVertices[0];
                firstVertex.setW(0);
                float originDistance = firstVertex.length();
                float originDistanceSquared = firstVertex.lengthSquared();

                for (int j = 1; j < scaledVertices.size(); ++j) {
                    Vector4 vertex = scaledVertices[j];
                    boundingBox.merge(vertex);
                    vertex.setW(0);
                    originDistance = qMax<float>(originDistance, vertex.length());
                    originDistanceSquared = qMax<float>(originDistance, vertex.lengthSquared());
                }

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

    QString getNewModelFilename(const QString &modelFilename) {
        QString newFilename = QDir::toNativeSeparators(modelFilename);
        if (newFilename.startsWith(QString("art") + QDir::separator(), Qt::CaseInsensitive)) {
            newFilename = newFilename.right(newFilename.length() - 4);
        }

        if (newFilename.endsWith(".skm", Qt::CaseInsensitive) || newFilename.endsWith(".ska", Qt::CaseInsensitive))
        {
            newFilename = newFilename.left(newFilename.length() - 4);
        }

        newFilename.append(".model");

        return newFilename;
    }

    bool openInput() {
        vfs.reset(new Troika::VirtualFileSystem());
        loadArchives();

        prototypes.reset(new Troika::Prototypes(vfs.data()));
        zoneTemplates.reset(new Troika::ZoneTemplates(vfs.data(), prototypes.data()));
        materials.reset(new Troika::Materials(vfs.data()));

        // Validate archives here?

        return true;
    }

    bool createOutputDirectory() {
        QDir outputDir(mOutputPath);
        if (!outputDir.exists()) {
            qDebug("Creating output directory %s.", qPrintable(mOutputPath));
            if (QDir::isAbsolutePath(mOutputPath)) {
                return QDir::root().mkpath(mOutputPath);
            } else {
                return QDir::current().mkpath(mOutputPath);
            }
        } else {
            return true;
        }
    }

    /**
      The interface files (*.tga and *.img in art/interface/) are all converted
      to consecutive PNG-24 images.
      */
    bool convertInterface() {

        ZipWriter zip(mOutputPath + "interface.zip");

        InterfaceConverter converter(&zip, vfs.data());

        return converter.convert();
    }

    /**
      Converts the particle systems.
      */
    bool convertParticleSystems() {
        qDebug("Converting particle systems.");

        QByteArray particleSystemData = vfs->openFile("rules/partsys0.tab")
                                        .append('\n')
                                        .append(vfs->openFile("rules/partsys1.tab"))
                                        .append('\n')
                                        .append(vfs->openFile("rules/partsys2.tab"));

        QDomDocument particleSystemsDoc;
        QDomElement particleSystems = particleSystemsDoc.createElement("particleSystems");
        particleSystemsDoc.appendChild(particleSystems);

        QList<QByteArray> lines = particleSystemData.split('\n');
        QHash<QByteArray, QList<QList<QByteArray> > > groupedEmitters; // Emitters grouped by particle system id

        foreach (QByteArray line, lines) {
            if (line.trimmed().isEmpty())
                continue; // Skip empty lines.

            line.replace((char)11, ""); // Replace "virtual tabs"

            QList<QByteArray> sections = line.split('\t');

            groupedEmitters[sections[0]].append(sections);
        }

        foreach (const QByteArray &particleSystemId, groupedEmitters.keys()) {
            particleSystemHashes[ELFHash(particleSystemId)] = particleSystemId;

            convertParticleSystem(particleSystemsDoc, particleSystems,
                                  particleSystemId, groupedEmitters[particleSystemId]);
        }

        ZipWriter writer(mOutputPath + "particles.zip");
        writer.addFile("particles/templates.xml", particleSystemsDoc.toByteArray(), 9);

        // Copy over a list of necessary textures
        QFile particleFiles(":/particlefiles.txt");

        if (!particleFiles.open(QIODevice::ReadOnly|QIODevice::Text)) {
            qWarning("Missing list of additional particle files.");
        }

        QStringList particleFileList = QString(particleFiles.readAll()).split('\n');

        foreach (QString particleFile, particleFileList) {
            particleFile = particleFile.trimmed();

            if (particleFile.startsWith('#') || particleFile.isEmpty())
                continue;

            QByteArray content = vfs->openFile("art/meshes/particle/" + particleFile);

            if (!content.isNull()) {
                writer.addFile("particles/" + particleFile, content, 9);
            } else {
                qWarning("Missing file: %s", qPrintable(particleFile));
            }
        }

        return true;
    }

    void convertParticleSystem(QDomDocument &document,
                               QDomElement &particleSystems,
                               const QString &id,
                               const QList<QList<QByteArray> > &emitters)
    {
        QDomElement particleSystem = document.createElement("particleSystem");
        particleSystems.appendChild(particleSystem);

        particleSystem.setAttribute("id", id);

        // Convert all emitters
        foreach (const QList<QByteArray> &sections, emitters) {
            convertParticleSystemEmitter(document, particleSystem, sections);
        }
    }

    QDomElement makeVector(QDomDocument &document, const QString &name, const QByteArray &x, const QByteArray &y, const QByteArray &z,
                           bool omitEmpty = true, bool omitZeros = false) {
        QDomElement element = document.createElement(name);
        if ((!omitZeros || QString(x) != "0") && (!omitEmpty || !x.isEmpty())) {
            element.setAttribute("x", QString(x));
        }
        if ((!omitZeros || QString(y) != "0") && (!omitEmpty || !y.isEmpty())) {
            element.setAttribute("y", QString(y));
        }
        if ((!omitZeros || QString(z) != "0") && (!omitEmpty || !z.isEmpty())) {
            element.setAttribute("z", QString(z));
        }
        return element;
    }

    bool isZeroOrEmpty(const QByteArray &value) {
        return value.trimmed().isEmpty() || value.trimmed() == "0";
    }

    void convertParticleSystemEmitter(QDomDocument &document, QDomElement &particleSystem,
                                      const QList<QByteArray> &sections)
    {
        QDomElement emitter = document.createElement("emitter");
        particleSystem.appendChild(emitter);

        emitter.setAttribute("name", QString(sections[1]));

        if (!isZeroOrEmpty(sections[2]))
            emitter.setAttribute("delay", QString(sections[2]));

        // Point emitters are standard. "Model Vert" is also possible, but no idea what it does
        QString type(sections[3]);
        if (!type.isEmpty() && type.toLower() != "point")
            emitter.setAttribute("type", type);

        QString lifespan(sections[4].trimmed().toLower());
        if (lifespan != "perm")
            emitter.setAttribute("lifespan", lifespan);

        QDomElement particles = document.createElement("particles");

        particles.setAttribute("rate", QString(sections[5]));

        if (!sections[11].isEmpty())
            particles.setAttribute("type", QString(sections[11]));
        if (!sections[12].isEmpty())
            particles.setAttribute("coordinateSystem", QString(sections[12]));
        //if (!sections[13].isEmpty())
        //    particles.setAttribute("positionCoordinates", QString(sections[13]));
        //if (!sections[14].isEmpty())
        //    particles.setAttribute("velocityCoordinates", QString(sections[14]));

        if (!sections[15].isEmpty())
            particles.setAttribute("material", "particles/" + QString(sections[15]) + ".tga");
        lifespan = QString(sections[16]).trimmed().toLower();
        if (!lifespan.isEmpty() && lifespan != "perm")
            particles.setAttribute("lifespan", lifespan);
        if (!sections[20].isEmpty())
            particles.setAttribute("model", "particles/" + QString(sections[20]) + ".model");

        if (!sections[17].isEmpty() && sections[17].toLower() != "add")
            emitter.setAttribute("blendMode", QString(sections[17]));
        if (!sections[6].isEmpty())
            emitter.setAttribute("boundingSphereRadius", QString(sections[6]));
        if (!sections[7].isEmpty())
            emitter.setAttribute("space", QString(sections[7]));
        if (!sections[8].isEmpty())
            emitter.setAttribute("spaceNode", QString(sections[8]));
        if (!sections[9].isEmpty())
            emitter.setAttribute("coordinateSystem", QString(sections[9]));
        if (!sections[10].isEmpty())
            emitter.setAttribute("offsetCoordinateSystem", QString(sections[10]));
        // if (!sections[18].isEmpty() && sections[18] != "0")
        //    emitter.setAttribute("bounce", QString(sections[18]));
        if (!sections[19].isEmpty())
            emitter.setAttribute("animationSpeed", QString(sections[19]));
        if (!sections[21].isEmpty())
            emitter.setAttribute("animation", QString(sections[21]));

        QDomElement element;

        // Emitter parameters
        if (!isZeroOrEmpty(sections[22]) || !isZeroOrEmpty(sections[23]) || !isZeroOrEmpty(sections[24])) {
            element = makeVector(document, "acceleration", sections[22], sections[23], sections[24], true, true);
            emitter.appendChild(element);
        }

        if (!isZeroOrEmpty(sections[25]) || !isZeroOrEmpty(sections[26]) || !isZeroOrEmpty(sections[27])) {
            element = makeVector(document, "velocity", sections[25], sections[26], sections[27], true, true);
            emitter.appendChild(element);
        }

        if (!isZeroOrEmpty(sections[28]) || !isZeroOrEmpty(sections[29]) || !isZeroOrEmpty(sections[30])) {
            element = makeVector(document, "position", sections[28], sections[29], sections[30], true, true);
            emitter.appendChild(element);
        }

        if (!isZeroOrEmpty(sections[31]) || !isZeroOrEmpty(sections[32]) || !isZeroOrEmpty(sections[33])) {
            element = document.createElement("rotation");
            if (!isZeroOrEmpty(sections[31]))
                element.setAttribute("yaw", QString(sections[31]));
            if (!isZeroOrEmpty(sections[32]))
                element.setAttribute("pitch", QString(sections[32]));
            if (!isZeroOrEmpty(sections[33]))
                element.setAttribute("roll", QString(sections[33]));
            emitter.appendChild(element);
        }

        // Scale was always the same for all components in all emitters
        // This value is used below to provide a default fallback for the particle scale
        if (!isZeroOrEmpty(sections[34]) || !isZeroOrEmpty(sections[35]) || !isZeroOrEmpty(sections[36])) {
            Q_ASSERT(sections[34] == sections[35] && sections[35] == sections[36]);
        }

        if (!isZeroOrEmpty(sections[37]) || !isZeroOrEmpty(sections[38]) || !isZeroOrEmpty(sections[39])) {
            element = makeVector(document, "offset", sections[37], sections[38], sections[39], true, true);
            emitter.appendChild(element);
        }

        if (!isZeroOrEmpty(sections[40]) || !isZeroOrEmpty(sections[41]) || !isZeroOrEmpty(sections[42])) {
            element = makeVector(document, "initialVelocity", sections[40], sections[41], sections[42], true, true);
            emitter.appendChild(element);
        }

        /**
          It is unclear what the purpose of this is.
          We use it as default values for the particle color that is defined below.
          */
        // element = document.createElement("initialColor");
        // element.setAttribute("red", QString(sections[44]));
        // element.setAttribute("green", QString(sections[45]));
        // element.setAttribute("blue", QString(sections[46]));
        // element.setAttribute("alpha", QString(sections[43]));
        // emitter.appendChild(element);

        // Particle parameters
        emitter.appendChild(particles);

        if (!isZeroOrEmpty(sections[47]) || !isZeroOrEmpty(sections[48]) || !isZeroOrEmpty(sections[49])) {
            element = makeVector(document, "acceleration", sections[47], sections[48], sections[49]);
            particles.appendChild(element);
        }

        if (!isZeroOrEmpty(sections[50]) || !isZeroOrEmpty(sections[51]) || !isZeroOrEmpty(sections[52])) {
            element = makeVector(document, "velocity", sections[50], sections[51], sections[52]);

            // Add coordinate-system type
            if (!sections[14].isEmpty() && sections[14].toLower() != "cartesian") {
                element.setAttribute("coordinates", QString(sections[14].toLower()));
            }

            particles.appendChild(element);
        }

        if (!isZeroOrEmpty(sections[53]) || !isZeroOrEmpty(sections[54]) || !isZeroOrEmpty(sections[55])) {
            element = makeVector(document, "position", sections[53], sections[54], sections[55]);

            // Add coordinate-system type
            if (!sections[13].isEmpty() && sections[13].toLower() != "cartesian") {
                element.setAttribute("coordinates", QString(sections[13].toLower()));
            }

            particles.appendChild(element);
        }

        if (!sections[56].isEmpty()) {
            QDomElement scaleChild = document.createElement("scale");
            scaleChild.appendChild(document.createTextNode(sections[56].trimmed()));
            particles.appendChild(scaleChild);
        } else if (!isZeroOrEmpty(sections[34])) {
            // Fall back to default scale from emitter
            QDomElement scaleChild = document.createElement("scale");
            scaleChild.appendChild(document.createTextNode(sections[34].trimmed()));
            particles.appendChild(scaleChild);
        }

        if (!isZeroOrEmpty(sections[59]) || !isZeroOrEmpty(sections[60]) || !isZeroOrEmpty(sections[61])) {
            element = document.createElement("rotation");
            if (!isZeroOrEmpty(sections[59]))
                element.setAttribute("yaw", QString(sections[59]));
            if (!isZeroOrEmpty(sections[60]))
                element.setAttribute("pitch", QString(sections[60]));
            if (!isZeroOrEmpty(sections[61]))
                element.setAttribute("roll", QString(sections[61]));
            particles.appendChild(element);
        }

        QByteArray colorRed = "255";
        QByteArray colorGreen = "255";
        QByteArray colorBlue = "255";
        QByteArray colorAlpha = "255";

        // Base emitter color?
        if (!isZeroOrEmpty(sections[44])) {
            colorRed = sections[44];
        }
        if (!isZeroOrEmpty(sections[45])) {
            colorGreen = sections[45];
        }
        if (!isZeroOrEmpty(sections[46])) {
            colorBlue = sections[46];
        }
        if (!isZeroOrEmpty(sections[43])) {
            colorAlpha = sections[43];
        }

        // Particle override colors ?
        if (!isZeroOrEmpty(sections[63])) {
            colorRed = sections[63];
        }
        if (!isZeroOrEmpty(sections[64])) {
            colorGreen = sections[64];
        }
        if (!isZeroOrEmpty(sections[65])) {
            colorBlue = sections[65];
        }
        if (!isZeroOrEmpty(sections[62])) {
            colorAlpha = sections[62];
        }

        QRegExp bracketRemover("\\(.+\\)");
        bracketRemover.setMinimal(true);

        element = document.createElement("color");
        element.setAttribute("red", QString(colorRed).replace(bracketRemover, ""));
        element.setAttribute("green", QString(colorGreen).replace(bracketRemover, ""));
        element.setAttribute("blue", QString(colorBlue).replace(bracketRemover, ""));
        element.setAttribute("alpha", QString(colorAlpha).replace(bracketRemover, ""));
        particles.appendChild(element);

        if (!sections[66].isEmpty()) {
            element = document.createElement("attractorBlend");
            element.appendChild(document.createTextNode(sections[66]));
            particles.appendChild(element);
        }
    }

    void addMeshesMesReferences()
    {
        QHash<quint32, QString> meshesIndex = Troika::MessageFile::parse(vfs->openFile("art/meshes/meshes.mes"));

        foreach (quint32 meshId, meshesIndex.keys()) {
            QString meshFilename = QDir::toNativeSeparators("art/meshes/" + meshesIndex[meshId] + ".skm");

            MeshReference &reference = meshReferences[meshFilename.toLower()];
            reference.meshId = meshId;
            reference.source |= MeshReference::MeshesMes;
        }
    }

    void addAddMeshesMesReferences()
    {
        QHash<quint32, QString> meshesIndex = Troika::MessageFile::parse(vfs->openFile("rules/addmesh.mes"));

        foreach (quint32 meshId, meshesIndex.keys()) {
            QString filename = meshesIndex[meshId].trimmed();

            if (filename.isEmpty())
                continue;

            QStringList filenames = filename.split(';');

            foreach (const QString &splitFilename, filenames) {
                MeshReference &reference = meshReferences[splitFilename];
                reference.source |= MeshReference::AddMesh;
            }
        }
    }

    void addHairReferences()
    {
        char genders[2] = { 'f', 'm' };
        const char *dir[2] = {"female", "male"};

        for (int style = 0; style < 8; ++style)  {
            for (int color = 0; color < 8; ++color) {
                for (int genderId = 0; genderId < 2; ++genderId) {
                    char gender = genders[genderId];

                    QString filename = QString("art/meshes/hair/%3/s%1/dw_%4_s%1_c%2_small.skm").arg(style).arg(color)
                                       .arg(dir[genderId]).arg(gender);
                    meshReferences[filename].source |= MeshReference::Hair;

                    filename = QString("art/meshes/hair/%3/s%1/ho_%4_s%1_c%2_small.skm").arg(style).arg(color)
                               .arg(dir[genderId]).arg(gender);
                    meshReferences[filename].source |= MeshReference::Hair;

                    filename = QString("art/meshes/hair/%3/s%1/hu_%4_s%1_c%2_small.skm").arg(style).arg(color)
                               .arg(dir[genderId]).arg(gender);
                    meshReferences[filename].source |= MeshReference::Hair;

                    if (style == 0 || style == 3 || (style == 4 && gender == 'f') || style == 6) {
                        filename = QString("art/meshes/hair/%3/s%1/dw_%4_s%1_c%2_big.skm").arg(style).arg(color)
                                   .arg(dir[genderId]).arg(gender);
                        meshReferences[filename].source |= MeshReference::Hair;

                        filename = QString("art/meshes/hair/%3/s%1/ho_%4_s%1_c%2_big.skm").arg(style).arg(color)
                                   .arg(dir[genderId]).arg(gender);
                        meshReferences[filename].source |= MeshReference::Hair;

                        filename = QString("art/meshes/hair/%3/s%1/hu_%4_s%1_c%2_big.skm").arg(style).arg(color)
                                   .arg(dir[genderId]).arg(gender);
                        meshReferences[filename].source |= MeshReference::Hair;
                    }

                    if (style == 5) {
                        filename = QString("art/meshes/hair/%3/s%1/dw_%4_s%1_c%2_none.skm").arg(style).arg(color)
                                   .arg(dir[genderId]).arg(gender);
                        meshReferences[filename].source |= MeshReference::Hair;

                        filename = QString("art/meshes/hair/%3/s%1/ho_%4_s%1_c%2_none.skm").arg(style).arg(color)
                                   .arg(dir[genderId]).arg(gender);
                        meshReferences[filename].source |= MeshReference::Hair;

                        filename = QString("art/meshes/hair/%3/s%1/hu_%4_s%1_c%2_none.skm").arg(style).arg(color)
                                   .arg(dir[genderId]).arg(gender);
                        meshReferences[filename].source |= MeshReference::Hair;
                    }
                }
            }
        }
    }

    void convertModels()
    {        
        ZipWriter zip(mOutputPath + "meshes.zip");

        // Convert all valid meshes in meshes.mes
        addMeshesMesReferences();
        addAddMeshesMesReferences();
        addHairReferences();

        int totalWork = meshReferences.size();
        int workDone = 0;

        foreach (const QString &meshFilename, meshReferences.keys()) {
            if (cancel)
                return;

            if (exclusions.isExcluded(meshFilename)) {
                qWarning("Skipping %s, since it's excluded.", qPrintable(meshFilename));
                workDone++;
                continue;
            }

            Troika::SkmReader reader(vfs.data(), materials.data(), meshFilename);
            QScopedPointer<Troika::MeshModel> model(reader.get());

            if (!model) {
                qWarning("Unable to open model %s.", qPrintable(meshFilename));
                workDone++;
                continue;
            }

            convertModel(&zip, meshFilename, model.data());

            updateProgress(++workDone, totalWork, 60, "Converting models");
        }

        convertMaterials(&zip);
    }       

    void convertMaterials(ZipWriter *zip)
    {
        QHash<uint,QString> materials = MessageFile::parse(vfs->openFile("rules/materials.mes"));

        QSet<QString> materialSet;

        foreach (QString value, materials.values()) {
            QStringList parts = value.split(':');
            if (!parts.size() == 2)
                continue;
            materialSet.insert(parts[1]);
        }

        QSet<QString> writtenTextures;
        QSet<QString> writtenMaterials;

        foreach (QString mdfFilename, materialSet) {
            MaterialConverter converter(vfs.data());
            converter.setExternal(true);
            converter.convert(Troika::Material::create(vfs.data(), mdfFilename));

            HashedData converted = converter.materialScripts().values().at(0);

            foreach (const QString &textureFilename, converter.textures().keys()) {
                if (writtenTextures.contains(textureFilename))
                    continue;
                writtenTextures.insert(textureFilename);
                zip->addFile(textureFilename, converter.textures()[textureFilename].data, 9);
            }

            foreach (const QString &materialFilename, converter.materialScripts().keys()) {
                if (writtenMaterials.contains(materialFilename))
                    continue;
                writtenMaterials.insert(materialFilename);
                zip->addFile(materialFilename, converter.materialScripts()[materialFilename].data, 9);
            }
        }

    }

    /**
      * Converts a single model, given its original filename.
      */
    bool convertModel(ZipWriter *zip, const QString &filename, Troika::MeshModel *model)
    {
        // This is a tedious process, but necessary. Create a total bounding box,
        // then create a bounding box for every animation of the mesh.
        QString newFilename = getNewModelFilename(filename);

        QByteArray modelData;
        QBuffer modelBuffer(&modelData);

        if (!modelBuffer.open(QIODevice::ReadWrite|QIODevice::Truncate)) {
            qFatal("Unable to open a write buffer for the model.");
            return false;
        }

        QDataStream stream(&modelBuffer);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        writeModel(model, stream, zip);

        modelBuffer.close();

        // writeDebugModel(model, newFilename + ".debug", zip);

        return zip->addFile(newFilename, modelData, 9);
    }

    // We convert the streams to non-interleaved data, which makes it easier to read them into vectors
    struct Streams {
        QMap<uint, QQuaternion> rotationFrames;
        QMap<uint, QVector3D> scaleFrames;
        QMap<uint, QVector3D> translationFrames;

        void appendCurrentState(const AnimationBoneState *state) {
            rotationFrames[state->rotationFrame] = state->rotation;
            scaleFrames[state->scaleFrame] = state->scale;
            translationFrames[state->translationFrame] = state->translation;
        }

        void appendNextState(const AnimationBoneState *state) {
            rotationFrames[state->nextRotationFrame] = state->nextRotation;
            scaleFrames[state->nextScaleFrame] = state->nextScale;
            translationFrames[state->nextTranslationFrame] = state->nextTranslation;
        }
    };

    void writeDebugModel(Troika::MeshModel *model, QString modelDir, ZipWriter *writer) {
        const Troika::Skeleton *skeleton = model->skeleton();

        QMap<uint, QString> animDataStartMap;

        foreach (const Troika::Animation &animation, skeleton->animations()) {

            if (animDataStartMap.contains(animation.keyFramesDataStart())) {
                continue;
            }

            animDataStartMap[animation.keyFramesDataStart()] = animation.name();

            QFile output(modelDir + "/" + animation.name() + ".txt");
            QDir current;
            current.mkpath(modelDir);

            output.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate);

            QTextStream stream(&output);
            stream.setRealNumberNotation(QTextStream::FixedNotation);
            stream.setRealNumberPrecision(5);
            stream.setFieldAlignment(QTextStream::AlignRight);

            // How do we ensure padding if the animations have variable length names?
            stream << "Name: " << animation.name().toUtf8() << endl;
            stream << "Frames: " << (uint)animation.frames() << endl << "Rate: " << animation.frameRate() << endl
                    << "DPS: " << animation.dps() << endl
                    << "Drive Type: " << (uint)animation.driveType() << endl
                    << "Loopable: " << animation.loopable() << endl << endl;

            foreach (const Troika::AnimationEvent &event, animation.events()) {
                stream << "Event @ " << event.frameId << " '" << event.type << "' '" << event.action << "'" << endl;
            }

            if (!animation.events().isEmpty())
                stream << endl;

            AnimationStream *animStream = animation.openStream(skeleton);

            QMap<uint,Streams> streams;

            // Write out the state of the first bones
            for (int i = 0; i < skeleton->bones().size(); ++i) {
                const AnimationBoneState *boneState = animStream->getBoneState(i);

                if (boneState) {
                    streams[i].appendCurrentState(boneState);
                }
            }
            int nextFrame = animStream->getNextFrameId();
            while (!animStream->atEnd()) {
                animStream->readNextFrame();
                if (animStream->getNextFrameId() <= nextFrame && !animStream->atEnd()) {
                    _CrtDbgBreak();
                }
                nextFrame = animStream->getNextFrameId();

                for (int i = 0; i < skeleton->bones().size(); ++i) {
                    const AnimationBoneState *boneState = animStream->getBoneState(i);

                    if (boneState) {
                        streams[i].appendCurrentState(boneState);
                    }
                }
            }

            // Also append the state of the last frame
            for (int i = 0; i < skeleton->bones().size(); ++i) {
                const AnimationBoneState *boneState = animStream->getBoneState(i);

                if (boneState) {
                    streams[i].appendNextState(boneState);
                }
            }

            animation.freeStream(animStream);

            // Write out the number of bones affected by the animation

            // At this point, we have the entire keyframe stream
            // IMPORTANT NOTE: Due to the use of QMap as the container, the keys will be guaranteed to be in ascending order for both bones and frames!
            foreach (uint boneId, streams.keys()) {
                // Write the keyframe streams for every bone
                const Streams &boneStreams = streams[boneId];
                stream << "Keyframes for bone " << model->skeleton()->bones()[boneId].name
                        << " (" << boneId << "):" << endl;

                stream << "Rotation:" << endl;
                foreach (uint frameId, boneStreams.rotationFrames.keys()) {
                    const QQuaternion &rotation = boneStreams.rotationFrames[frameId];
                    stream << " " << qSetFieldWidth(5) << (quint16)frameId << qSetFieldWidth(0)
                            << ": " << qSetFieldWidth(12) << rotation.x() << rotation.y() << rotation.z()
                            << rotation.scalar() << qSetFieldWidth(0) << endl;
                }
                stream << endl << "Scale:" << endl;
                foreach (uint frameId, boneStreams.scaleFrames.keys()) {
                    const QVector3D &scale = boneStreams.scaleFrames[frameId];
                    stream << " " << qSetFieldWidth(5) << (quint16)frameId << qSetFieldWidth(0) << ": "
                            << qSetFieldWidth(12) << scale.x() << scale.y() << scale.z() << qSetFieldWidth(0) << endl;
                }
                stream << endl << "Translation:" << endl;
                foreach (uint frameId, boneStreams.translationFrames.keys()) {
                    const QVector3D &translation = boneStreams.translationFrames[frameId];
                    stream << " " << qSetFieldWidth(5) << (quint16)frameId << qSetFieldWidth(0) << ": "
                            << qSetFieldWidth(12) << translation.x() << translation.y()
                            << translation.z() << qSetFieldWidth(0) << endl;
                }
                stream << endl << endl;
            }

            stream << "Bone Hierarchy:" << endl;
            // Write out the bone hieararchy
            stream.setFieldAlignment(QTextStream::AlignLeft);
            for (int i = 0; i < model->skeleton()->bones().size(); ++i) {
                const Troika::Bone &bone = model->skeleton()->bones()[i];
                if (bone.parentId == -1) {
                    writeDebugBoneHierarchy(stream, bone, model->skeleton()->bones());
                }
            }
            stream.setFieldAlignment(QTextStream::AlignRight);

        }
    }

    void writeDebugBoneHierarchy(QTextStream &stream, const Bone &bone, const QVector<Bone> &bones, int indent = 0) {

        for (int i = 0; i < indent; ++i)
            stream << "  ";

        stream << qSetFieldWidth(4) << bone.id << qSetFieldWidth(0) << " " << bone.name << " (Flags: " << bone.flags
                << ")" << endl;

        for (int i = 0; i < bones.size(); ++i)
            if (bones[i].parentId == bone.id)
                writeDebugBoneHierarchy(stream, bones[i], bones, indent + 1);
    }

    bool writeModel(Troika::MeshModel *model, QDataStream &stream, ZipWriter *zip)
    {
        ModelWriter writer(stream);

        QHash< QString, QSharedPointer<Troika::Material> > groupedMaterials;

        MaterialConverter converter(vfs.data());
        converter.setExternal(external);

        // Convert materials used by the model
        foreach (const QSharedPointer<Troika::FaceGroup> &faceGroup, model->faceGroups()) {
            if (!faceGroup->material())
                continue;
            groupedMaterials[faceGroup->material()->name()] = faceGroup->material();
        }

        QHash<QString,int> materialMapping;
        QHash<uint,QString> materialFileMapping;
        int i = 0;
        int j = -1; // placeholders are negative
        QStringList placeholders;

        foreach (QString materialName, groupedMaterials.keys()) {

            if (groupedMaterials[materialName]->type() == Material::Placeholder) {
                placeholders.append(materialName);
                materialMapping[materialName] = (j--);
            } else {
                materialFileMapping[i] = getNewMaterialFilename(materialName);
                converter.convert(groupedMaterials[materialName].data());
                materialMapping[materialName] = i++;
            }

        }

        if (!external) {
            writer.writeTextures(converter.textureList());
            writer.writeMaterials(converter.materialList(), placeholders);
        } else {
            writer.writeMaterials(QList<HashedData>(), placeholders);

            QStringList materials;
            for (int j = 0; j < i; ++j) {
                materials.append(materialFileMapping[j]);
            }
            writer.writeMaterialReferences(materials);

            foreach (const QString &filename, converter.materialScripts().keys()) {
                if (!mWrittenMaterials[filename]) {
                    mWrittenMaterials[filename] = true;
                    zip->addFile(filename, converter.materialScripts()[filename].data, 9);
                }
            }
            foreach (const QString &filename, converter.textures().keys()) {
                if (!mWrittenTextures[filename]) {
                    mWrittenTextures[filename] = true;
                    zip->addFile(filename, converter.textures()[filename].data, 9);
                }
            }
        }

        writer.writeVertices(model->vertices());
        writer.writeFaces(model->faceGroups(), materialMapping);

        writer.writeBones(model->skeleton());
        writer.writeBoneAttachments(model->vertices());

        writer.writeBoundingVolumes(model);

        if (model->skeleton()->animations().size() > 0) {
            writer.writeAnimations(model);
        }

        writer.finish();

        return true;
    }

    /**
      Converts scripts and global definition files.
      */
    void convertScripts()
    {
        ZipWriter writer(mOutputPath + "scripts.zip");

        QScriptEngine engine;

        QScriptValue readMesFn = engine.newFunction((QScriptEngine::FunctionWithArgSignature)readMesFile, vfs.data());
        engine.globalObject().setProperty("readMes", readMesFn);

        QScriptValue readTabFn = engine.newFunction((QScriptEngine::FunctionWithArgSignature)readTabFile, vfs.data());
        engine.globalObject().setProperty("readTab", readTabFn);

        QScriptValue addFileFn = engine.newFunction((QScriptEngine::FunctionWithArgSignature)addFile, &writer);
        engine.globalObject().setProperty("addFile", addFileFn);

        QFile scriptFile("scripts/converter.js");

        if (!scriptFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
            qFatal("Unable to open converter script: scripts/converter.js");
        }

        QString scriptCode = scriptFile.readAll();

        engine.evaluate(scriptCode, "scripts/converter.js");

        convertPrototypes(&writer, &engine);

        writer.close();
    }

    void convertTranslations()
    {
        ZipWriter writer(mOutputPath + "translations.zip");

        QByteArray result;
        QDataStream stream(&result, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        QStringList mesFiles = vfs->listAllFiles("*.mes");

        foreach (const QString &mesFile, mesFiles) {
            QString mesKey = QDir::toNativeSeparators(mesFile).replace(QDir::separator(), "/");
            mesKey.replace(QRegExp("\\.mes$"), "").toLower();

            if (!mesKey.startsWith("mes/")) {
                continue;
            }

            QHash<uint,QString> entries = MessageFile::parse(vfs->openFile(mesFile));

            foreach (uint key, entries.keys()) {
                stream << QString("%1/%2").arg(mesKey).arg(key) << entries[key];
            }
        }

        writer.addFile("translation.dat", result, 9);
    }

    void convertPrototypes(ZipWriter *writer, QScriptEngine *engine)
    {
        PrototypeConverter converter(vfs.data());

        QVariantMap result = converter.convertPrototypes(prototypes.data());

        QScriptValue postprocess = engine->globalObject().property("postprocess");

        QScriptValueList args;
        args.append(engine->toScriptValue<QVariantMap>(result));

        postprocess.call(QScriptValue(), args);

        if (engine->hasUncaughtException()) {
            qWarning("JS Error: %s", qPrintable(engine->uncaughtException().toString()));
            return;
        }
    }

    void updateProgress(int innerSectionValue, int innerSectionTotal, int sectionWorth, const QString &section)
    {
        int currentProgress = 100 * (sectionsDone + (innerSectionValue / (float)innerSectionTotal) * sectionWorth);
        int totalProgress = totalWorkSections * 100;

        if (lastProgressUpdate == currentProgress) {
            return;
        }

        emit converter->progressUpdate(currentProgress, totalProgress, section);
    }

    bool convert()
    {
        if (!openInput()) {
            qFatal("Unable to open Temple of Elemental Evil data files in %s.", qPrintable(mInputPath));
        }

        if (!createOutputDirectory()) {
            qFatal("Unable to create output directory %s.", qPrintable(mOutputPath));
        }

        mInternalDescription = MessageFile::parse(vfs->openFile("oemes/oname.mes"));

        emit converter->progressUpdate(sectionsDone, totalWorkSections, "Converting scripts");

        convertScripts();

        if (cancel)
            return false;

        convertTranslations();

        if (cancel)
            return false;

        emit converter->progressUpdate(sectionsDone, totalWorkSections, "Converting particle systems");

        convertParticleSystems();
        sectionsDone += 5;

        if (cancel)
            return false;

        emit converter->progressUpdate(sectionsDone, totalWorkSections, "Converting interface");

        convertInterface();
        sectionsDone += 10;

        emit converter->progressUpdate(sectionsDone, totalWorkSections, "Converting maps");

        convertMaps();
        sectionsDone += 25;

        if (cancel)
            return false;

        emit converter->progressUpdate(sectionsDone, totalWorkSections, "Converting models");

        convertModels();
        sectionsDone += 60;

        if (cancel)
            return false;

        emit converter->progressUpdate(sectionsDone, totalWorkSections, "Finished");

        if (cancel)
            return false;

        return true;
    }

};

Converter::Converter(const QString &inputPath, const QString &outputPath) :
        d_ptr(new ConverterData(this, inputPath, outputPath))
{
}

Converter::~Converter()
{
}

bool Converter::convert()
{
    return d_ptr->convert();
}

void Converter::setExternal(bool ext)
{
    d_ptr->external = ext;
}

void Converter::cancel()
{
    d_ptr->cancel = true;
}

bool Converter::isCancelled() const
{
    return d_ptr->cancel;
}

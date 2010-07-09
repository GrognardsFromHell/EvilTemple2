
#include <QTextStream>
#include <QDataStream>
#include <QVector>
#include <QHash>
#include <QMap>

#include <model.h>
#include <troika_material.h>
#include <virtualfilesystem.h>
#include <skmreader.h>

#include "util.h"
#include "convertmodelstask.h"
#include "materialconverter.h"
#include "modelwriter.h"

void writeDebugModel(IFileWriter *writer, Troika::MeshModel *model, const QString &modelDir);
void writeDebugBoneHierarchy(QTextStream &stream,
                             const Troika::Bone &bone,
                             const QVector<Troika::Bone> &bones,
                             int indent = 0);
bool writeModel(IFileWriter *writer, Troika::MeshModel *model, QDataStream &stream);

ConvertModelsTask::ConvertModelsTask(IConversionService *service, QObject *parent)
    : ConversionTask(service, parent)
{
    if (!mExclusions.load(":/exclusions.txt")) {
        qWarning("Unable to load mesh exclusions.");
    }
}

uint ConvertModelsTask::cost() const
{
    return 30;
}

QString ConvertModelsTask::description() const
{
    return "Converting models";
}

void ConvertModelsTask::run()
{
    // Convert all valid meshes in meshes.mes
    addMeshesMesReferences();
    addAddMeshesMesReferences();
    addHairReferences();

    convertReferencedMeshes();
}

void ConvertModelsTask::addHairReferences()
{
    char genders[2] = { 'f', 'm' };
    const char *dir[2] = {"female", "male"};

    for (int style = 0; style < 8; ++style)  {
        for (int color = 0; color < 8; ++color) {
            for (int genderId = 0; genderId < 2; ++genderId) {
                char gender = genders[genderId];

                QString filename = QString("art/meshes/hair/%3/s%1/dw_%4_s%1_c%2_small.skm").arg(style).arg(color)
                                   .arg(dir[genderId]).arg(gender);
                service()->addMeshReference(filename);

                filename = QString("art/meshes/hair/%3/s%1/ho_%4_s%1_c%2_small.skm").arg(style).arg(color)
                           .arg(dir[genderId]).arg(gender);
                service()->addMeshReference(filename);

                filename = QString("art/meshes/hair/%3/s%1/hu_%4_s%1_c%2_small.skm").arg(style).arg(color)
                           .arg(dir[genderId]).arg(gender);
                service()->addMeshReference(filename);

                if (style == 0 || style == 3 || (style == 4 && gender == 'f') || style == 6) {
                    filename = QString("art/meshes/hair/%3/s%1/dw_%4_s%1_c%2_big.skm").arg(style).arg(color)
                               .arg(dir[genderId]).arg(gender);
                    service()->addMeshReference(filename);

                    filename = QString("art/meshes/hair/%3/s%1/ho_%4_s%1_c%2_big.skm").arg(style).arg(color)
                               .arg(dir[genderId]).arg(gender);
                    service()->addMeshReference(filename);

                    filename = QString("art/meshes/hair/%3/s%1/hu_%4_s%1_c%2_big.skm").arg(style).arg(color)
                               .arg(dir[genderId]).arg(gender);
                    service()->addMeshReference(filename);
                }

                if (style == 5) {
                    filename = QString("art/meshes/hair/%3/s%1/dw_%4_s%1_c%2_none.skm").arg(style).arg(color)
                               .arg(dir[genderId]).arg(gender);
                    service()->addMeshReference(filename);

                    filename = QString("art/meshes/hair/%3/s%1/ho_%4_s%1_c%2_none.skm").arg(style).arg(color)
                               .arg(dir[genderId]).arg(gender);
                    service()->addMeshReference(filename);

                    filename = QString("art/meshes/hair/%3/s%1/hu_%4_s%1_c%2_none.skm").arg(style).arg(color)
                               .arg(dir[genderId]).arg(gender);
                    service()->addMeshReference(filename);
                }
            }
        }
    }
}


void ConvertModelsTask::addMeshesMesReferences()
{
    QHash<uint, QString> meshesIndex = service()->openMessageFile("art/meshes/meshes.mes");

    foreach (quint32 meshId, meshesIndex.keys()) {
        QString meshFilename = QDir::toNativeSeparators("art/meshes/" + meshesIndex[meshId] + ".skm");

        service()->addMeshReference(meshFilename);
    }
}

void ConvertModelsTask::addAddMeshesMesReferences()
{
    QHash<uint, QString> meshesIndex = service()->openMessageFile("art/meshes/addmesh.mes");

    foreach (quint32 meshId, meshesIndex.keys()) {
        QString filename = meshesIndex[meshId].trimmed();

        if (filename.isEmpty())
            continue;

        QStringList filenames = filename.split(';');

        foreach (const QString &splitFilename, filenames) {
            service()->addMeshReference(splitFilename);
        }
    }
}


void ConvertModelsTask::convertReferencedMeshes()
{
    QScopedPointer<IFileWriter> output(service()->createOutput("meshes"));

    const QSet<QString> &meshReferences = service()->getMeshReferences();

    int totalWork = meshReferences.size();
    int workDone = 0;

    foreach (const QString &meshFilename, meshReferences) {
        assertNotAborted();

        ++workDone;

        if (mExclusions.isExcluded(meshFilename)) {
            qWarning("Skipping %s, since it's excluded.", qPrintable(meshFilename));
            workDone++;
            continue;
        }

        Troika::SkmReader reader(service()->virtualFileSystem(), service()->materials(), meshFilename);
        QScopedPointer<Troika::MeshModel> model(reader.get());

        if (!model) {
            qWarning("Unable to open model %s.", qPrintable(meshFilename));
            workDone++;
            continue;
        }

        convertModel(output.data(), meshFilename, model.data());

        if (workDone % 10 == 0)
            emit progress(workDone, totalWork);
    }

    convertMaterials(output.data());

    output->close();
}

void ConvertModelsTask::convertMaterials(IFileWriter *output)
{
    QHash<uint,QString> materials = service()->openMessageFile("rules/materials.mes");

    QSet<QString> materialSet;

    foreach (QString value, materials.values()) {
        QStringList parts = value.split(':');
        if (parts.size() != 2)
            continue;
        materialSet.insert(parts[1]);
    }

    QSet<QString> writtenTextures;
    QSet<QString> writtenMaterials;

    foreach (QString mdfFilename, materialSet) {
        MaterialConverter converter(service()->virtualFileSystem());
        converter.setExternal(true);
        converter.convert(Troika::Material::create(service()->virtualFileSystem(), mdfFilename));

        HashedData converted = converter.materialScripts().values().at(0);

        foreach (const QString &textureFilename, converter.textures().keys()) {
            if (writtenTextures.contains(textureFilename))
                continue;
            writtenTextures.insert(textureFilename);
            output->addFile(textureFilename, converter.textures()[textureFilename].data, 9);
        }

        foreach (const QString &materialFilename, converter.materialScripts().keys()) {
            if (writtenMaterials.contains(materialFilename))
                continue;
            writtenMaterials.insert(materialFilename);
            output->addFile(materialFilename, converter.materialScripts()[materialFilename].data, 9);
        }
    }

}

/**
  * Converts a single model, given its original filename.
  */
bool ConvertModelsTask::convertModel(IFileWriter *output, const QString &filename, Troika::MeshModel *model)
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

    writeModel(output, model, stream);

    modelBuffer.close();

    // writeDebugModel(model, newFilename + ".debug", zip);

    output->addFile(newFilename, modelData);
}

// We convert the streams to non-interleaved data, which makes it easier to read them into vectors
struct Streams {
    QMap<uint, QQuaternion> rotationFrames;
    QMap<uint, QVector3D> scaleFrames;
    QMap<uint, QVector3D> translationFrames;

    void appendCurrentState(const Troika::AnimationBoneState *state) {
        rotationFrames[state->rotationFrame] = state->rotation;
        scaleFrames[state->scaleFrame] = state->scale;
        translationFrames[state->translationFrame] = state->translation;
    }

    void appendNextState(const Troika::AnimationBoneState *state) {
        rotationFrames[state->nextRotationFrame] = state->nextRotation;
        scaleFrames[state->nextScaleFrame] = state->nextScale;
        translationFrames[state->nextTranslationFrame] = state->nextTranslation;
    }
};

static void writeDebugModel(IFileWriter *writer, Troika::MeshModel *model, const QString &modelDir)
{
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

        Troika::AnimationStream *animStream = animation.openStream(skeleton);

        QMap<uint,Streams> streams;

        // Write out the state of the first bones
        for (int i = 0; i < skeleton->bones().size(); ++i) {
            const Troika::AnimationBoneState *boneState = animStream->getBoneState(i);

            if (boneState) {
                streams[i].appendCurrentState(boneState);
            }
        }
        int nextFrame = animStream->getNextFrameId();
        while (!animStream->atEnd()) {
            animStream->readNextFrame();
            Q_ASSERT(animStream->getNextFrameId() > nextFrame || animStream->atEnd());
            nextFrame = animStream->getNextFrameId();

            for (int i = 0; i < skeleton->bones().size(); ++i) {
                const Troika::AnimationBoneState *boneState = animStream->getBoneState(i);

                if (boneState) {
                    streams[i].appendCurrentState(boneState);
                }
            }
        }

        // Also append the state of the last frame
        for (int i = 0; i < skeleton->bones().size(); ++i) {
            const Troika::AnimationBoneState *boneState = animStream->getBoneState(i);

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

static void writeDebugBoneHierarchy(QTextStream &stream,
                                    const Troika::Bone &bone,
                                    const QVector<Troika::Bone> &bones,
                                    int indent)
{

    for (int i = 0; i < indent; ++i)
        stream << "  ";

    stream << qSetFieldWidth(4) << bone.id << qSetFieldWidth(0) << " " << bone.name << " (Flags: " << bone.flags
            << ")" << endl;

    for (int i = 0; i < bones.size(); ++i)
        if (bones[i].parentId == bone.id)
            writeDebugBoneHierarchy(stream, bones[i], bones, indent + 1);
}

bool ConvertModelsTask::writeModel(IFileWriter *output, Troika::MeshModel *model, QDataStream &stream)
{
    const bool external = false;

    ModelWriter writer(stream);

    QHash< QString, QSharedPointer<Troika::Material> > groupedMaterials;

    MaterialConverter converter(service()->virtualFileSystem());
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
        if (groupedMaterials[materialName]->type() == Troika::Material::Placeholder) {
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
                output->addFile(filename, converter.materialScripts()[filename].data, 9);
            }
        }
        foreach (const QString &filename, converter.textures().keys()) {
            if (!mWrittenTextures[filename]) {
                mWrittenTextures[filename] = true;
                output->addFile(filename, converter.textures()[filename].data, 9);
            }
        }
    }

    writer.writeVertices(model->vertices());
    writer.writeFaces(model->faceGroups(), materialMapping);

    writer.writeBones(model->skeleton());
    writer.writeBoneAttachments(model->vertices());

    writer.writeBoundingVolumes(model);

    if (!model->skeleton()->animations().isEmpty())
        writer.writeAnimations(model);

    writer.finish();

    return true;
}


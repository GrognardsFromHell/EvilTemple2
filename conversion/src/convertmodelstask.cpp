
#include <QtConcurrentRun>
#include <QTextStream>
#include <QDataStream>
#include <QVector>
#include <QHash>
#include <QMap>
#include <QApplication>

#include <troika_model.h>
#include <troika_material.h>
#include <virtualfilesystem.h>
#include <skmreader.h>

#include "conversion/convertmodelstask.h"
#include "conversion/materialconverter.h"
#include "conversion/modelwriter.h"
#include "conversion/util.h"

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
        QString meshFilename = normalizePath("art/meshes/" + meshesIndex[meshId] + ".skm");

        service()->addMeshReference(meshFilename);
    }
}

void ConvertModelsTask::addAddMeshesMesReferences()
{
    QHash<uint, QString> meshesIndex = service()->openMessageFile("rules/addmesh.mes");

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

    mWorkDone = 0;

    QList< QFuture<bool> > futures;

    foreach (QString meshFilename, meshReferences) {
        assertNotAborted();

        meshFilename = normalizePath(meshFilename);

        futures.append(QtConcurrent::run(this, &ConvertModelsTask::convertModel, output.data(), meshFilename));
    }

    // Now start waiting on the futures
    int lastReport = 0;
    for (int i = 0; i < futures.size(); ++i) {
        QFuture<bool> future = futures[i];
        if (!future.isFinished()) {
            --i;
            QThread::yieldCurrentThread();
        }

        int workNow = mWorkDone;
        if (workNow - lastReport > 10) {
            emit progress(workNow, meshReferences.size());
            lastReport = workNow;
            QApplication::processEvents();
        }
    }

    assertNotAborted();

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

        foreach (const QString &textureFilename, converter.textures().keys()) {
            if (writtenTextures.contains(textureFilename))
                continue;
            writtenTextures.insert(textureFilename);
            output->addFile(textureFilename, converter.textures()[textureFilename].data);
        }

        foreach (const QString &materialFilename, converter.materialScripts().keys()) {
            if (writtenMaterials.contains(materialFilename))
                continue;
            writtenMaterials.insert(materialFilename);
            output->addFile(materialFilename, converter.materialScripts()[materialFilename].data);
        }
    }

}

/**
  * Converts a single model, given its original filename.
  */
bool ConvertModelsTask::convertModel(IFileWriter *output, const QString &filename)
{
    if (isAborted())
        return false;

    if (mExclusions.isExcluded(filename)) {
        qWarning("Skipping %s, since it's excluded.", qPrintable(filename));
        mWorkDone.fetchAndAddOrdered(1);
        return false;
    }

    Troika::SkmReader reader(service()->virtualFileSystem(), service()->materials(), filename);
    QScopedPointer<Troika::MeshModel> model(reader.get());

    if (!model) {
        qWarning("Unable to open model %s.", qPrintable(filename));
        mWorkDone.fetchAndAddOrdered(1);
        return false;
    }

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

    writeModel(filename, output, model.data(), stream);

    modelBuffer.close();

    // writeDebugModel(model, newFilename + ".debug", zip);

    output->addFile(newFilename, modelData);

    mWorkDone.fetchAndAddOrdered(1);

    return true;
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

bool ConvertModelsTask::writeModel(const QString &filename, IFileWriter *output,
                                   Troika::MeshModel *model, QDataStream &stream)
{
    const bool external = true;

    ModelWriter writer(filename, stream);

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
    QVector<QByteArray> placeholders;

    foreach (QString materialName, groupedMaterials.keys()) {
        if (groupedMaterials[materialName]->type() == Troika::Material::Placeholder) {
            placeholders.append(materialName.toLatin1());
            materialMapping[materialName] = (j--);
        } else {
            materialFileMapping[i] = getNewMaterialFilename(materialName);
            converter.convert(groupedMaterials[materialName].data());
            materialMapping[materialName] = i++;
        }
    }

    if (!external) {
        writer.writeTextures(converter.textureList());
        writer.writeMaterials(converter.materialList());
        if (!placeholders.isEmpty())
            writer.writeMaterialPlaceholders(placeholders);
    } else {
        QStringList materials;
        for (int j = 0; j < i; ++j) {
            materials.append(materialFileMapping[j]);
        }
        writer.writeMaterialReferences(materials);

        if (!placeholders.isEmpty())
            writer.writeMaterialPlaceholders(placeholders);

        foreach (const QString &filename, converter.materialScripts().keys()) {
            if (!mWrittenMaterials[filename]) {
                mWrittenMaterials[filename] = true;
                output->addFile(filename, converter.materialScripts()[filename].data);
            }
        }
        foreach (const QString &filename, converter.textures().keys()) {
            if (!mWrittenTextures[filename]) {
                mWrittenTextures[filename] = true;
                output->addFile(filename, converter.textures()[filename].data);
            }
        }
    }

    writer.writeVertices(model->vertices());
    writer.writeFaces(model->faceGroups(), materialMapping);

    if (!model->skeleton()->animations().isEmpty()) {
        writer.writeSkeleton(model->skeleton());
        writer.writeBindingPose(model);
        writer.writeBoundingVolumes(model);
        writer.writeAnimations(model);
    } else {
        writer.writeBindingPose(model);
        writer.writeBoundingVolumes(model);
    }

    writer.finish();

    return true;
}


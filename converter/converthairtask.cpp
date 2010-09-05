#include <QDataStream>
#include <QByteArray>
#include <QStringList>

#include "converthairtask.h"
#include "modelwriter.h"

#include <troika_model.h>
#include <virtualfilesystem.h>
#include <skmreader.h>

static const uint style_count = 8;

static const char *female_styles[] = {
    "long",
    "ponytail",
    "short",
    "topknot",
    "pigtails",
    "braids",
    "mohawk",
    "ponytail2"
};

static const char *male_styles[] = {
    "long",
    "ponytail",
    "short",
    "topknot",
    "mullet",
    "bald",
    "mohawk",
    "medium"
};

static const char *genderPaths[] = {
    "male/",
    "female/"
};

static const char genderShorts[] = {
    'm',
    'f'
};

static const char *racePathIn[] = {
    "dw", // Dwarven model
    "hu", // Human model
    "ho" // Half-orc model
};

static const char *racePathOut[] = {
    "dwarf", // Dwarven model
    "human", // Human model
    "halforc" // Half-orc model
};

static const char *types[] = {
    "none",
    "small",
    "big"
};

static const QString hairMaterial("meshes/hair/material.xml");

ConvertHairTask::ConvertHairTask(IConversionService *service, QObject *parent)
    : ConversionTask(service, parent)
{
}

static bool convertHairModel(IConversionService *service,
                             IFileWriter *writer,
                             const QString &inputPath,
                             const QString &outputPath)
{
    Troika::SkmReader reader(service->virtualFileSystem(), service->materials(), inputPath);

    QScopedPointer<Troika::MeshModel> model(reader.get());

    if (!model) {
        qWarning("Unable to open model %s.", qPrintable(inputPath));
        return false;
    }

    QByteArray modelData;
    QDataStream stream(&modelData, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    QHash<QString, int> materialMapping;
    for (int i = 0; i < model->faceGroups().length(); ++i) {
        // All materials map to the new hair material (at position 0)
        materialMapping.insert(model->faceGroups().at(i)->material()->name(), 0);
    }

    QStringList hairMaterials;
    hairMaterials.append(hairMaterial);

    ModelWriter modelWriter(outputPath, stream);
    modelWriter.writeMaterialReferences(hairMaterials);
    modelWriter.writeVertices(model->vertices());
    modelWriter.writeFaces(model->faceGroups(), materialMapping);
    modelWriter.writeBindingPose(model.data());
    modelWriter.writeBoundingVolumes(model.data());
    modelWriter.finish();

    writer->addFile(outputPath, modelData);

    return true;
}

void ConvertHairTask::run()
{
    QScopedPointer<IFileWriter> writer(service()->createOutput("hair"));

    int filesConverted = 0;

    for (int gender = 0; gender < 2; ++gender) {
        QString genderPath = QString("art/meshes/Hair/%1").arg(genderPaths[gender]);
        char genderShort = genderShorts[gender];

        for (int race = 0; race < 3; ++race) {
            for (int style = 0; style < style_count; ++style) {

                QString styleName = (gender == 0) ? male_styles[style] : female_styles[style];

                for (int type = 0; type < 3; ++type) {
                    QString typeName = types[type];

                    QString inputPath = QString("%1s%4/%2_%3_s%4_c0_%5.skm")
                                        .arg(genderPath)
                                        .arg(racePathIn[race])
                                        .arg(genderShort)
                                        .arg(style)
                                        .arg(typeName);

                    if (!service()->virtualFileSystem()->exists(inputPath))
                        continue;

                    QString outputPath = QString("meshes/hair/%1/%2-%3/%4.model")
                                         .arg(styleName)
                                         .arg(racePathOut[race])
                                         .arg((gender == 0) ? "male" : "female")
                                         .arg(typeName);

                    convertHairModel(service(), writer.data(), inputPath, outputPath);
                    filesConverted++;
                }
            }
        }
    }

    writer->close();

    qDebug("Hair files converted: %d", filesConverted);
}

uint ConvertHairTask::cost() const
{
    return 1;
}

QString ConvertHairTask::description() const
{
        return "Converting hair styles.";
}

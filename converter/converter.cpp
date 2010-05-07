
#include <QtGlobal>
#include <QtXml>
#include <QDataStream>
#include <QScopedPointer>
#include <QFileInfo>
#include <QTextStream>
#include <QBuffer>

#include <iostream>

#include "zipwriter.h"

#include "virtualfilesystem.h"
#include "messagefile.h"
#include "troikaarchive.h"
#include "prototypes.h"
#include "materials.h"
#include "zonetemplate.h"
#include "zonetemplatereader.h"
#include "zonetemplates.h"
#include "zonebackgroundmap.h"
#include "skmreader.h"
#include "model.h"
#include "material.h"
#include "exclusions.h"

#include "util.h"
#include "converter.h"
#include "materialconverter.h"
#include "interfaceconverter.h"
#include "modelwriter.h"

struct MeshReference {
    MeshReference() : meshId(0), source(0) {}

    enum Sources {
        MeshesMes = 0x1,
        StaticGeometry = 0x2, // from a map
        FORCE_DWORD = 0x7FFFFFFF, // force 32-bit size (whatever)
    };

    uint meshId; // From meshes.mes
    uint source; // bitfield
};

class ConverterData {
public:
    static const int maxArchives = 10; // Try ToEE0.dat to ToEE(maxArchives-1).dat

    QString mInputPath, mOutputPath;

    QScopedPointer<Troika::VirtualFileSystem> vfs;

    QScopedPointer<Troika::Prototypes> prototypes;

    QScopedPointer<Troika::ZoneTemplates> zoneTemplates;

    QScopedPointer<Troika::Materials> materials;

    QScopedPointer<ZipWriter> commonFiles;

    // Maps lower-case mesh filenames (normalized separators) to an information structure
    QHash<QString, MeshReference> meshReferences;

    Exclusions exclusions;

    ConverterData(const QString &inputPath, const QString &outputPath) :
            mInputPath(inputPath), mOutputPath(outputPath)
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
        // Convert all maps
        foreach (quint32 mapId, zoneTemplates->mapIds()) {
            QScopedPointer<Troika::ZoneTemplate> zoneTemplate(zoneTemplates->load(mapId));

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
            } else {
                qWarning("Unable to load zone template for map id %d.", mapId);
            }
        }

        return true;
    }

    QString getNewModelFilename(const QString &modelFilename) {
        QString newFilename = modelFilename;
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

    bool convertMaterials() {

        // Search for all mdf files
        QStringList materialFiles = vfs->listAllFiles("*.mdf");

        ZipWriter writer(mOutputPath + "materials.zip");

        foreach (const QString &materialFile, materialFiles) {
            qDebug("Converting material %s.", qPrintable(materialFile));
            MaterialConverter converter(vfs.data(), &writer, materialFile);
            if (!converter.convert()) {
                qWarning("Failed conversion of %s.", qPrintable(materialFile));
            }
        }

        return true;
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

    /**
      At the moment, this method only dumps a CSV with information about models.
      */
    void convertModels()
    {        
        ZipWriter zip(mOutputPath + "meshes.zip");

        // Convert all valid meshes in meshes.mes
        addMeshesMesReferences();

        foreach (const QString &meshFilename, meshReferences.keys()) {
            if (exclusions.isExcluded(meshFilename)) {
                qWarning("Skipping %s, since it's excluded.", qPrintable(meshFilename));
                continue;
            }

            Troika::SkmReader reader(vfs.data(), materials.data(), meshFilename);
            QScopedPointer<Troika::MeshModel> model(reader.get());

            if (!model) {
                qWarning("Unable to open model %s.", qPrintable(meshFilename));
                continue;
            }

            convertModel(&zip, meshFilename, model.data());
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

        writeModel(model, stream);

        modelBuffer.close();

        return zip->addFile(newFilename, modelData, 9);
    }

    bool writeModel(Troika::MeshModel *model, QDataStream &stream)
    {
        ModelWriter writer(stream);

        writer.writeVertices(model->vertices());
        writer.writeFaces(model->faceGroups());

        writer.finish();

        return true;
    }

    bool convert()
    {
        if (!openInput()) {
            qFatal("Unable to open Temple of Elemental Evil data files in %s.", qPrintable(mInputPath));
        }

        if (!createOutputDirectory()) {
            qFatal("Unable to create output directory %s.", qPrintable(mOutputPath));
        }

        convertMaps();

        convertModels();

        convertInterface();

        return true;
    }

};

Converter::Converter(const QString &inputPath, const QString &outputPath) :
        d_ptr(new ConverterData(inputPath, outputPath))
{
}

Converter::~Converter()
{
}

bool Converter::convert()
{
    return d_ptr->convert();
}

int main(int argc, char **argv) {

    QCoreApplication app(argc, argv);

    std::cout << "Conversion utility for Temple of Elemental Evil." << std::endl;

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " " << "<install-dir>" << std::endl;
        return -1;
    }

    Converter converter(QString::fromLocal8Bit(argv[1]), QString("data/"));

    if (!converter.convert()) {
        std::cout << "ERROR: Conversion failed." << std::endl;
        return -1;
    }

    return 0;
}

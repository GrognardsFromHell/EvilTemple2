#ifndef CONVERTMODELSTASK_H
#define CONVERTMODELSTASK_H

#include <QString>
#include <QDataStream>
#include <QHash>

#include "conversiontask.h"
#include "exclusions.h"

namespace Troika {
    class MeshModel;
}

class ConvertModelsTask : public ConversionTask
{
public:
    explicit ConvertModelsTask(IConversionService *service, QObject *parent = 0);

    void run();

    uint cost() const;

    QString description() const;

private:
    Exclusions mExclusions;

    void addMeshesMesReferences();
    void addAddMeshesMesReferences();
    void addHairReferences();

    void convertReferencedMeshes();

    void convertMaterials(IFileWriter *writer);
    bool convertModel(IFileWriter *output, const QString &filename);
    bool writeModel(const QString &filename, IFileWriter *output, Troika::MeshModel *model, QDataStream &stream);

    QHash<QString, bool> mWrittenTextures;

    QHash<QString, bool> mWrittenMaterials;

    QAtomicInt mWorkDone;
};

#endif // CONVERTMODELSTASK_H

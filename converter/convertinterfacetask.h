#ifndef CONVERTINTERFACETASK_H
#define CONVERTINTERFACETASK_H

#include <QStringList>

#include "conversiontask.h"

class ConvertInterfaceTask : public ConversionTask
{
Q_OBJECT
public:
    ConvertInterfaceTask(IConversionService *service, QObject *parent = NULL);

    void run();

    uint cost() const;

    QString description() const;

private:
    void convertTextures(IFileWriter *writer);
    void convertImages(IFileWriter *writer);
    void convertImage(IFileWriter *writer, const QString &baseName, int width, int height);

    QStringList mExcludePaths; // Paths to exclude in the texture conversion step

};

#endif // CONVERTINTERFACETASK_H

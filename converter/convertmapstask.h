#ifndef CONVERTMAPSTASK_H
#define CONVERTMAPSTASK_H

#include "conversiontask.h"
#include "exclusions.h"

class ConvertMapsTask : public ConversionTask
{
Q_OBJECT
public:
    ConvertMapsTask(IConversionService *service, QObject *parent = NULL);

    void run();

    uint cost() const;

    QString description() const;

private:
    Exclusions mMapExclusions;

    void convertStaticObjects(Troika::ZoneTemplate *zoneTemplate, IFileWriter *writer);
};

#endif // CONVERTMAPSTASK_H

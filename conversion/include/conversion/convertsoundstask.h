#ifndef CONVERTSOUNDSTASK_H
#define CONVERTSOUNDSTASK_H

#include "global.h"

#include "conversiontask.h"

class CONVERSIONSHARED_EXPORT ConvertSoundsTask : public ConversionTask
{
public:
    ConvertSoundsTask(IConversionService *service, QObject *parent = NULL);

    void run();

    uint cost() const;

    QString description() const;
};

#endif // CONVERTSOUNDSTASK_H

#ifndef CONVERTSOUNDSTASK_H
#define CONVERTSOUNDSTASK_H

#include "conversiontask.h"

class ConvertSoundsTask : public ConversionTask
{
public:
    ConvertSoundsTask(IConversionService *service, QObject *parent = NULL);

    void run();

    uint cost() const;

    QString description() const;
};

#endif // CONVERTSOUNDSTASK_H

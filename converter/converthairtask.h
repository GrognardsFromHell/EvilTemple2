#ifndef CONVERTHAIRTASK_H
#define CONVERTHAIRTASK_H

#include "conversiontask.h"

class ConvertHairTask : public ConversionTask
{
Q_OBJECT
public:
    ConvertHairTask(IConversionService *service, QObject *parent = 0);

    void run();

    uint cost() const;

    QString description() const;

};

#endif // CONVERTHAIRTASK_H

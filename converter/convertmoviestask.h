#ifndef CONVERTMOVIESTASK_H
#define CONVERTMOVIESTASK_H

#include "conversiontask.h"

class ConvertMoviesTask : public ConversionTask
{
public:
    ConvertMoviesTask(IConversionService *service, QObject *parent = NULL);

    void run();

    uint cost() const;

    QString description() const;
};


#endif // CONVERTMOVIESTASK_H

#ifndef CONVERTMOVIESTASK_H
#define CONVERTMOVIESTASK_H

#include "global.h"

#include "conversiontask.h"

class CONVERSIONSHARED_EXPORT ConvertMoviesTask : public ConversionTask
{
public:
    ConvertMoviesTask(IConversionService *service, QObject *parent = NULL);

    void run();

    uint cost() const;

    QString description() const;
};


#endif // CONVERTMOVIESTASK_H

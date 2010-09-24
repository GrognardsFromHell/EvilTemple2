#ifndef CONVERTPARTICLESYSTEMSTASK_H
#define CONVERTPARTICLESYSTEMSTASK_H

#include "global.h"

#include "conversiontask.h"

class CONVERSIONSHARED_EXPORT ConvertParticleSystemsTask : public ConversionTask
{
public:
    explicit ConvertParticleSystemsTask(IConversionService *service, QObject *parent = 0);

    void run();

    uint cost() const;

    QString description() const;
};

#endif // CONVERTPARTICLESYSTEMSTASK_H

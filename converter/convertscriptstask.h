#ifndef CONVERTSCRIPTSTASK_H
#define CONVERTSCRIPTSTASK_H

#include "conversiontask.h"

class QScriptEngine;

class ConvertScriptsTask : public ConversionTask
{
Q_OBJECT
public:
    ConvertScriptsTask(IConversionService *service, QObject *parent = 0);

    void run();

    uint cost() const;

    QString description() const;

private:
    void convertPrototypes(IFileWriter *writer, QScriptEngine *engine);

};

#endif // CONVERTSCRIPTSTASK_H

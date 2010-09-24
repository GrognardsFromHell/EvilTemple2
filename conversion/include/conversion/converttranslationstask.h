#ifndef CONVERTTRANSLATIONSTASK_H
#define CONVERTTRANSLATIONSTASK_H

#include "global.h"

#include "conversiontask.h"

class CONVERSIONSHARED_EXPORT ConvertTranslationsTask : public ConversionTask
{
public:
    explicit ConvertTranslationsTask(IConversionService *service, QObject *parent = 0);

    void run();

    uint cost() const;

    QString description() const;
};

#endif // CONVERTTRANSLATIONSTASK_H

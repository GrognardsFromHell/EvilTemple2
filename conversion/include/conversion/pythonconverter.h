#ifndef PYTHONCONVERTER_H
#define PYTHONCONVERTER_H

#include "global.h"

#include <QString>
#include <QByteArray>

#include "conversiontask.h"

class CONVERSIONSHARED_EXPORT PythonConverter
{
public:
    PythonConverter(IConversionService *service);

    QString convert(const QByteArray &code, const QString &filename);

    QString convertDialogGuard(const QByteArray &code, const QString &filename);

    QString convertDialogAction(const QByteArray &code, const QString &filename);

private:
    IConversionService *mService;
};

#endif // PYTHONCONVERTER_H

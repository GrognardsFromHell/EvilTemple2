#ifndef PYTHONCONVERTER_H
#define PYTHONCONVERTER_H

#include <QString>
#include <QByteArray>

#include "conversiontask.h"

class PythonConverter
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

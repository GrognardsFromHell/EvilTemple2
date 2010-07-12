#ifndef PYTHONCONVERTER_H
#define PYTHONCONVERTER_H

#include <QString>
#include <QByteArray>

class PythonConverter
{
public:
    PythonConverter();

    QString convert(const QByteArray &code, const QString &filename);

    QString convertDialogGuard(const QByteArray &code, const QString &filename);

    QString convertDialogAction(const QByteArray &code, const QString &filename);
};

#endif // PYTHONCONVERTER_H

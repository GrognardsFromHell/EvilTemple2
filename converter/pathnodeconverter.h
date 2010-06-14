#ifndef PATHNODECONVERTER_H
#define PATHNODECONVERTER_H

#include <QScopedPointer>
#include <QVariantMap>
#include <QString>
#include <QByteArray>

class PathNodeConverterData;

class PathNodeConverter
{
public:
    PathNodeConverter();
    ~PathNodeConverter();

    bool load(const QByteArray &data);

    const QString &error() const;

    QVariantMap convert();

private:
    QScopedPointer<PathNodeConverterData> d;
};

#endif // PATHNODECONVERTER_H

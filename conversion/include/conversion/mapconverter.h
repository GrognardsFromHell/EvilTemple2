#ifndef MAPCONVERTER_H
#define MAPCONVERTER_H

#include "global.h"

#include <QHash>
#include <QString>

#include "conversiontask.h"

#include "zonetemplate.h"
using namespace Troika;

class CONVERSIONSHARED_EXPORT MapConverter
{
public:
    MapConverter(IConversionService *service, IFileWriter *writer);

    bool convert(const ZoneTemplate *zoneTemplate);

private:
    QString convertGroundMap(const ZoneBackgroundMap *background);

    IConversionService *mService;
    IFileWriter *mWriter;
    QHash<QString,QString> convertedGroundMaps;

    Troika::VirtualFileSystem *mVfs;
};

#endif // MAPCONVERTER_H

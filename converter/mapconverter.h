#ifndef MAPCONVERTER_H
#define MAPCONVERTER_H

#include <QHash>
#include <QScopedPointer>

#include "virtualfilesystem.h"
#include "zonetemplate.h"
#include "zipwriter.h"

using namespace Troika;

class MapConverterData;

class MapConverter
{
public:
    MapConverter(VirtualFileSystem *vfs, ZipWriter *writer);
    ~MapConverter();

    bool convert(const ZoneTemplate *zoneTemplate);

private:
    QScopedPointer<MapConverterData> d_ptr;
};

#endif // MAPCONVERTER_H

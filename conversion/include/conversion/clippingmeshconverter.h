#ifndef CLIPPINGMESHCONVERTER_H
#define CLIPPINGMESHCONVERTER_H

#include <QScopedPointer>

#include "global.h"

namespace Troika {
    class ZoneTemplate;
}

class ClippingMeshConverterData;
class IConversionService;

class CONVERSIONSHARED_EXPORT ClippingMeshConverter
{
public:
    explicit ClippingMeshConverter(IConversionService *service, const Troika::ZoneTemplate *zoneTemplate);
    ~ClippingMeshConverter();

public:
    QByteArray convert();

private:
    QScopedPointer<ClippingMeshConverterData> d;
};

#endif // CLIPPINGMESHCONVERTER_H

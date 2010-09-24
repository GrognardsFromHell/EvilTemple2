#ifndef SECTORCONVERTER_H
#define SECTORCONVERTER_H

#include <QImage>
#include <QByteArray>

#include <zonetemplate.h>

#include "global.h"

/**
  Converts blocking / visibility information for a map.
  */
class CONVERSIONSHARED_EXPORT SectorConverter
{
public:
    SectorConverter(const Troika::ZoneTemplate *zoneTemplate);

    QByteArray convert();

    QImage createWalkableImage() const;
    QImage createGroundMaterialImage() const;
    QImage createHeightImage() const;
    QImage createVisionExtendImage() const;
    QImage createVisionBaseImage() const;
    QImage createVisionArchwayImage() const;
    QImage createVisionEndImage() const;

private:
    const Troika::ZoneTemplate *mZoneTemplate;

};

#endif // SECTORCONVERTER_H

#ifndef PROTOTYPECONVERTER_H
#define PROTOTYPECONVERTER_H

#include "prototypes.h"

#include <QDomDocument>

class PrototypeConverter
{
public:

    QDomDocument convertPrototypes(Troika::Prototypes *prototypes);

};

#endif // PROTOTYPECONVERTER_H

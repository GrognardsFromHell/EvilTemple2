#include "prototypeconverter.h"
#include "prototypes.h"

using namespace Troika;

static QDomElement convertPrototype(Prototype *prototype, QDomDocument &document)
{
    QDomElement ePrototype = document.createElement("prototype");

    ePrototype.setAttribute("id", prototype->id);

    return ePrototype;
}

QDomDocument PrototypeConverter::convertPrototypes(Troika::Prototypes *prototypes)
{
    QDomDocument document;
    QDomElement ePrototypes = document.createElement("prototypes");
    document.appendChild(ePrototypes);

    foreach (Prototype *prototype, prototypes->prototypes()) {
        QDomElement ePrototype = convertPrototype(prototype, document);
        ePrototypes.appendChild(ePrototype);
    }

    return document;
}

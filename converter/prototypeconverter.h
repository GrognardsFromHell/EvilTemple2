#ifndef PROTOTYPECONVERTER_H
#define PROTOTYPECONVERTER_H

#include "prototypes.h"
#include "virtualfilesystem.h"

#include <QXmlStreamWriter>
#include <QHash>

class PrototypeConverter
{
public:

    PrototypeConverter(Troika::VirtualFileSystem *vfs);

    void convertPrototypes(Troika::Prototypes *prototypes, QXmlStreamWriter &xml);

    void convertPrototype(Troika::Prototype *prototype, QXmlStreamWriter &xml);

private:

    QHash<uint, QString> mInternalDescriptions;
    QHash<uint, QString> mDescriptions;

    Troika::VirtualFileSystem *mVfs;

};

#endif // PROTOTYPECONVERTER_H

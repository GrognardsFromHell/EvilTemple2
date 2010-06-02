#ifndef PROTOTYPECONVERTER_H
#define PROTOTYPECONVERTER_H

#include "prototypes.h"
#include "virtualfilesystem.h"

#include <QVariantMap>
#include <QHash>

class PrototypeConverter
{
public:

    PrototypeConverter(Troika::VirtualFileSystem *vfs);

    QVariantMap convertPrototypes(Troika::Prototypes *prototypes);

    QVariantMap convertPrototype(Troika::Prototype *prototype);

private:

    QHash<uint, QString> mInternalDescriptions;
    QHash<uint, QString> mDescriptions;
    QHash<uint, QString> mModelFiles;

    Troika::VirtualFileSystem *mVfs;

};

#endif // PROTOTYPECONVERTER_H

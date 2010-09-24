#ifndef PROTOTYPECONVERTER_H
#define PROTOTYPECONVERTER_H

#include "global.h"

#include "prototypes.h"
#include "virtualfilesystem.h"

#include <QVariantMap>
#include <QHash>

class CONVERSIONSHARED_EXPORT PrototypeConverter
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

#ifndef OBJECTFILEREADER_H
#define OBJECTFILEREADER_H

#include "troikaformatsglobal.h"

#include <QDataStream>
#include <QHash>
#include <QString>

namespace Troika
{

    class ObjectFileReaderData;
    class Prototypes;
    class GeometryMeshObject;
    class VirtualFileSystem;
    class Materials;
    class Models;
    class GeometryObject;

    const int ObjectFileVersion = 0x77;

    class TROIKAFORMATS_EXPORT ObjectFileReader
    {
    public:
        ObjectFileReader(Prototypes *prototypes, QDataStream &stream);
        ~ObjectFileReader();

        bool read(bool skipHeader = false);
        const QString &errorMessage() const;

        GeometryObject *createObject(QHash<uint,QString> meshMapping);
    private:
        QScopedPointer<ObjectFileReaderData> d_ptr;

        Q_DISABLE_COPY(ObjectFileReader);
    };

}

#endif // OBJECTFILEREADER_H

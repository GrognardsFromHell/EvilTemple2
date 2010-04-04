#ifndef OBJECTFILEREADER_H
#define OBJECTFILEREADER_H

#include <QDataStream>

namespace EvilTemple
{

    class ObjectFileReaderData;
    class Prototypes;
    class GeometryMeshObject;
    class VirtualFileSystem;
    class Materials;
    class Models;

    const int ObjectFileVersion = 0x77;

    class ObjectFileReader
    {
    public:
        ObjectFileReader(Prototypes *prototypes, QDataStream &stream);
        ~ObjectFileReader();

        bool read(bool skipHeader = false);
        const QString &errorMessage() const;
        GeometryMeshObject *createMeshObject(Models *models);
    private:
        QScopedPointer<ObjectFileReaderData> d_ptr;

        Q_DISABLE_COPY(ObjectFileReader);
    };

}

#endif // OBJECTFILEREADER_H

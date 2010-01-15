#ifndef SKMREADER_H
#define SKMREADER_H

#include <QString>
#include <QSharedPointer>

#include "modelsource.h"

namespace EvilTemple
{

    class VirtualFileSystem;
    class Materials;
    class MeshModel;
    class SkmReaderData;

    class SkmReader : public ModelSource
    {
    public:
        SkmReader(VirtualFileSystem *vfs, Materials *materials, const QString &filename);
        ~SkmReader();

        QSharedPointer<MeshModel> get();

    private:
        QScopedPointer<SkmReaderData> d_ptr;
    };

}

#endif // SKMREADER_H

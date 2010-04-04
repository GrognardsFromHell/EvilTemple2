#ifndef DAGREADER_H
#define DAGREADER_H

#include <QString>
#include <QSharedPointer>

#include "modelsource.h"

namespace EvilTemple
{

    class VirtualFileSystem;
    class MeshModel;
    class DagReaderData;

    class DagReader : public ModelSource
    {
    public:
        DagReader(VirtualFileSystem *vfs, const QString &filename);
        ~DagReader();

        QSharedPointer<MeshModel> get();

    private:
        QScopedPointer<DagReaderData> d_ptr;
    };

}

#endif // DAGREADER_H

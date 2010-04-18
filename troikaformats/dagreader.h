#ifndef DAGREADER_H
#define DAGREADER_H

#include "troikaformatsglobal.h"

#include <QString>
#include <QSharedPointer>

#include "modelsource.h"

namespace Troika
{

    class VirtualFileSystem;
    class MeshModel;
    class DagReaderData;

    class TROIKAFORMATS_EXPORT DagReader : public ModelSource
    {
    public:
        DagReader(VirtualFileSystem *vfs, const QString &filename);
        ~DagReader();

        MeshModel *get();

    private:
        QScopedPointer<DagReaderData> d_ptr;
    };

}

#endif // DAGREADER_H

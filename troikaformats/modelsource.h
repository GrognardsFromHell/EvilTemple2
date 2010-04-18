#ifndef MODELSOURCE_H
#define MODELSOURCE_H

#include "troikaformatsglobal.h"

#include <QSharedPointer>

namespace Troika
{

    class MeshModel;

    /**
        Interface that represents the mesh source for a geometry mesh object.
     */
    class TROIKAFORMATS_EXPORT ModelSource
    {
    public:
        /**
          Loads the mesh model.
          */
        virtual MeshModel *get() = 0;
    };

};

#endif // MODELSOURCE_H

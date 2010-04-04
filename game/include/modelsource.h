#ifndef MODELSOURCE_H
#define MODELSOURCE_H

#include <QSharedPointer>

namespace EvilTemple
{

    class MeshModel;

    /**
        Interface that represents the mesh source for a geometry mesh object.
     */
    class ModelSource
    {
    public:
        /**
          Loads the mesh model.
          */
        virtual QSharedPointer<MeshModel> get() = 0;
    };

};

#endif // MODELSOURCE_H

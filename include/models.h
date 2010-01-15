#ifndef MODELS_H
#define MODELS_H

#include <QObject>
#include <QSharedPointer>

#include "modelsource.h"
#include "model.h"

namespace EvilTemple
{

    class Materials;
    class VirtualFileSystem;
    class MeshModel;
    class ModelsData;

    /**
      This class maintains a cache of all currently used models in order to avoid loading a model
      twice. References to models are held using a weak pointer, so no real caching beyond re-using
      already loaded models is performed.
      */
    class Models : public QObject
    {
        Q_OBJECT
    public:
        explicit Models(VirtualFileSystem *vfs, Materials *materials, QObject *parent = 0);
        ~Models();

    signals:

    public slots:

        /**
          Gets a model based on a legacy model id.
          */
        QSharedPointer<MeshModel> get(quint16 modelId);

        /**
          Loads a model using it's SKM filename. The SKA skeleton is
          automatically loaded as well.
          */
        QSharedPointer<MeshModel> get(const QString &filename);

    private:
        QScopedPointer<ModelsData> d_ptr;

        Q_DISABLE_COPY(Models);

    };

    class LegacyModelSource : public ModelSource
    {
    public:
        LegacyModelSource(Models *models, quint32 modelId);

        QSharedPointer<MeshModel> get();
    private:
        Models *_models;
        quint32 _modelId;
    };

    class FileModelSource : public ModelSource
    {
    public:
        FileModelSource(Models *models, const QString &filename);

        QSharedPointer<MeshModel> get();
    private:
        Models *_models;
        QString _filename;
    };

}

#endif // MODELS_H

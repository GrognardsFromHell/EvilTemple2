
#include "io/virtualfilesystem.h"
#include "io/messagefile.h"
#include "io/skmreader.h"
#include "materials.h"
#include "model.h"
#include "models.h"

#include <QHash>

namespace EvilTemple
{

    typedef QHash< QString, QWeakPointer<MeshModel> > ModelCache;

    class ModelsData
    {
    private:
        VirtualFileSystem *vfs;
        Materials *materials;

        /**
          Maps the legacy model id to a full SKM filename.
          */
        QHash<quint32, QString> legacyModels;

        /**
          Maps SKM filenames to MeshModels.
          */
        ModelCache cache;

        /**
          Loads the legacy mapping from a numeric identifier to a model filename.
          */
        void loadLegacyModelMapping()
        {
            QByteArray data = vfs->openFile("art/meshes/meshes.mes");

            if (data.isNull()) {
                qWarning("Legacy meshes mapping is missing: art/meshes/meshes.mes");
                return;
            }

            legacyModels = MessageFile::parse(data);
        }

    public:
        ModelsData(VirtualFileSystem *_vfs, Materials *_materials) : vfs(_vfs), materials(_materials)
        {
            loadLegacyModelMapping();
        }

        QSharedPointer<MeshModel> get(quint16 modelId)
        {
            QHash<quint32, QString>::iterator it = legacyModels.find(modelId);

            if (it == legacyModels.end()) {
                qWarning("Unknown legacy model id: %d", modelId);
                return QSharedPointer<MeshModel>();
            }

            QString modelFilename = QString("art/meshes/%1.skm").arg(it.value());

            return get(modelFilename);
        }

        /**
          Loads a model either from the cache or from the disk.

          @param filename The SKM filename.
          */
        QSharedPointer<MeshModel> get(const QString &filename)
        {
            QString lowerFilename = filename.toLower();

            ModelCache::iterator it = cache.find(lowerFilename);

            // We found an entry in the cache
            if (it != cache.end()) {
                // Promote the weak-ref to a shared pointer before testing validity
                QSharedPointer<MeshModel> result(*it);

                if (result) {
                    // The pointer is still good, return it
                    return result;
                }
            }

            SkmReader reader(vfs, materials, filename);
            QSharedPointer<MeshModel> result(reader.get());

            cache[lowerFilename] = result.toWeakRef(); // Keep a weak reference

            return result;
        }
    };

    Models::Models(VirtualFileSystem *vfs, Materials *materials, QObject *parent) :
        QObject(parent), d_ptr(new ModelsData(vfs, materials))
    {
    }

    Models::~Models()
    {
    }

    QSharedPointer<MeshModel> Models::get(quint16 modelId)
    {
        return d_ptr->get(modelId);
    }

    QSharedPointer<MeshModel> Models::get(const QString &filename)
    {
        return d_ptr->get(filename);
    }

    LegacyModelSource::LegacyModelSource(Models *models, quint32 modelId)
        : _models(models), _modelId(modelId)
    {
    }

    QSharedPointer<MeshModel> LegacyModelSource::get()
    {
        return _models->get(_modelId);
    }

    FileModelSource::FileModelSource(Models *models, const QString &filename)
        : _models(models), _filename(filename)
    {
    }

    QSharedPointer<MeshModel> FileModelSource::get()
    {
        return _models->get(_filename);
    }

}

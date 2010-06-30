
#include <QDir>

#include "models.h"

namespace EvilTemple {

typedef QWeakPointer<Model> WeakSharedModel;
typedef QHash<QString, WeakSharedModel> ModelCache;

class ModelsData
{
public:
    ModelsData(Materials *_materials, RenderStates &_renderStates) :
            materials(_materials), renderStates(_renderStates)
    {
    }

    Materials *materials;
    RenderStates &renderStates;

    ModelCache cache;
};

static inline QString normalizeFilename(const QString &filename)
{
    QString result = QDir::toNativeSeparators(filename);
    return QDir::cleanPath(result);
}

Models::Models(Materials *materials, RenderStates &renderStates, QObject *parent)
    : QObject(parent), d(new ModelsData(materials, renderStates))
{
}

Models::~Models()
{
}

SharedModel Models::load(const QString &filename)
{
    QString cleanedFilename = normalizeFilename(filename);

    ModelCache::const_iterator it = d->cache.find(cleanedFilename);

    if (it != d->cache.end()) {
        SharedModel cachedEntry(it.value());

        /*
         This double-check becomes necessary, since the weak pointer may already be invalid.
         */
        if (cachedEntry)
            return cachedEntry;
    }

    SharedModel result(new Model);

    if (!result->load(cleanedFilename, d->materials, d->renderStates)) {
        qWarning("Unable to load model %s: %s.", qPrintable(cleanedFilename), result->error());

        // TODO: Fall back to a default model.
    }

    d->cache[cleanedFilename] = result;

    return result;
}

}

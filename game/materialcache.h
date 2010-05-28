
#ifndef MATERIALCACHE_H
#define MATERIALCACHE_H

#include <QtCore/QHash>

#include "materialstate.h"

namespace EvilTemple {

class GlobalMaterialCacheCleanupThread;
class GlobalMaterialCache {
friend class GlobalMaterialCacheCleanupThread;
public:
    static GlobalMaterialCache &instance() {
        return mInstance;
    }

    /**
      Tries to retrieve a texture from the texture cache by its Md5 hash.
      Returns a null pointer if no such texture exists in the cache.
      */
    SharedMaterialState get(const QString &filename);

    /**
      Loads a material from disk, stores it in the cache and returns it. If the
      material is already loaded, its retrieved from the cache and returned instead.
      */
    SharedMaterialState load(const QString &filename, RenderStates &renderStates);

    /**
      Inserts a texture into the cache.
      */
    void insert(const QString &filename, const SharedMaterialState &texture);

private:
    GlobalMaterialCache();
    ~GlobalMaterialCache();

    typedef QHash<QString, QWeakPointer<MaterialState> > CacheContainer;
    typedef CacheContainer::iterator iterator;

    CacheContainer mMaterials;

    QMutex mCleanupMutex;
    GlobalMaterialCacheCleanupThread *mThread;
    static GlobalMaterialCache mInstance;
};

inline SharedMaterialState loadMaterial(const QString &filename, RenderStates &renderStates)
{
    SharedMaterialState result = GlobalMaterialCache::instance().get(filename);

    if (!result) {
        result = SharedMaterialState(new MaterialState);
        if (result->createFromFile(filename, renderStates, FileTextureSource::instance())) {
            GlobalMaterialCache::instance().insert(filename, result);
        } else {
            qWarning("Unable to load material %s.", qPrintable(filename));
        }
    }

    return result;
}

}

#endif // MATERIALCACHE_H

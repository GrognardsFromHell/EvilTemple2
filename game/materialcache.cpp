
#include "materialcache.h"

#include <QFile>

namespace EvilTemple {

class GlobalMaterialCacheCleanupThread : public QThread
{
public:
    const static uint MaterialPruneInterval = 2500; // Milliseconds

    GlobalMaterialCacheCleanupThread(GlobalMaterialCache &cache)
        : mCache(cache)
    {
    }

    void cancel()
    {
        QMutexLocker locker(&mWaitMutex);
        mWaitCondition.wakeAll();
    }

protected:
    void run()
    {
        QMutexLocker locker(&mWaitMutex);
        while (!mWaitCondition.wait(&mWaitMutex, MaterialPruneInterval)) {
            pruneMaterials();
        }
    }

private:

    void pruneMaterials()
    {
        QMutexLocker locker(&mCache.mCleanupMutex);

        GlobalMaterialCache::iterator it = mCache.mMaterials.begin();

        while (it != mCache.mMaterials.end()) {
            if (it.value().isNull()) {
                it = mCache.mMaterials.erase(it);
            } else {
                it++;
            }
        }
    }

    GlobalMaterialCache &mCache;
    QMutex mWaitMutex;
    QWaitCondition mWaitCondition;
};

GlobalMaterialCache::GlobalMaterialCache() : mThread(new GlobalMaterialCacheCleanupThread(*this))
{
    mThread->start();
}

GlobalMaterialCache::~GlobalMaterialCache()
{
    mThread->cancel();
    mThread->wait();
    delete mThread;
}

SharedMaterialState GlobalMaterialCache::get(const QString &filename)
{
    QMutexLocker locker(&mCleanupMutex);

    iterator it = mMaterials.find(filename);

    if (it != mMaterials.end()) {
        return SharedMaterialState(it.value());
    } else {
        return SharedMaterialState(0);
    }
}

void GlobalMaterialCache::insert(const QString &filename, const SharedMaterialState &texture)
{
    QMutexLocker locker(&mCleanupMutex);

    mMaterials[filename] = QWeakPointer<MaterialState>(texture);
}

SharedMaterialState GlobalMaterialCache::load(const QString &filename, RenderStates &renderStates)
{
    QMutexLocker locker(&mCleanupMutex);

    if (mMaterials.contains(filename)) {
        return mMaterials[filename];
    } else {
        SharedMaterialState material(new MaterialState);

        if (!material->createFromFile(filename, renderStates, FileTextureSource::instance())) {
            qWarning("Unable to open material file %s: %s", qPrintable(filename),
                     qPrintable(material->error()));
        }

        mMaterials[filename] = material;

        return material;
    }
}

GlobalMaterialCache GlobalMaterialCache::mInstance;

}

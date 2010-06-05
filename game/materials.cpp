
#include <QSharedPointer>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

#include "materials.h"

namespace EvilTemple {

class MaterialsCleanupThread : public QThread
{
public:
    const static uint MaterialPruneInterval = 2500; // Milliseconds

    MaterialsCleanupThread(MaterialsData *data) : d(data), mCancelled(false)
    {
    }

    void cancel()
    {
        mCancelled = true;
    }

protected:
    void run()
    {
        QMutexLocker locker(&mWaitMutex);
        while (!mWaitCondition.wait(&mWaitMutex, MaterialPruneInterval) && !mCancelled) {
            pruneMaterials();
        }
    }

private:

    void pruneMaterials();

    volatile bool mCancelled;
    MaterialsData *d;
    QMutex mWaitMutex;
    QWaitCondition mWaitCondition;
};

class MaterialsData
{
public:
    MaterialsData(RenderStates &_renderStates)
        : renderStates(_renderStates), thread(this) {}

    typedef QHash<QString, QWeakPointer<MaterialState> > CacheContainer;
    typedef CacheContainer::iterator iterator;

    QString error;
    SharedMaterialState missingMaterial;
    CacheContainer activeMaterials;

    QWaitCondition waitCondition;
    QMutex cleanupMutex;
    QMutex waitMutex; // Used to wait for cancellation
    MaterialsCleanupThread thread;
    RenderStates &renderStates;
};

void MaterialsCleanupThread::pruneMaterials()
{
    QMutexLocker locker(&d->cleanupMutex);

    MaterialsData::iterator it = d->activeMaterials.begin();

    int pruned = 0;

    while (it != d->activeMaterials.end()) {
        if (it.value().isNull()) {
            it = d->activeMaterials.erase(it);
            pruned++;
        } else {
            it++;
        }
    }

    if (pruned > 0) {
        qDebug("Pruned %d materials from the cache.", pruned);
    }
}

Materials::Materials(RenderStates &renderStates, QObject *parent) :
    QObject(parent), d(new MaterialsData(renderStates))
{
}

Materials::~Materials()
{
    qDebug("Shutting down cleanup thread for materials cache.");
    QMutexLocker locker(&d->waitMutex);
    d->thread.cancel();
    d->waitCondition.wakeAll();
    d->thread.wait();
}

bool Materials::load()
{

    if (!d->missingMaterial->createFromFile(":/material/missing_material.xml", d->renderStates)) {
        d->error = "Unable to load the default material for missing materials: ";
        d->error.append(d->missingMaterial->error());
    }

    return false;
}

const QString &Materials::error() const
{
    return d->error;
}

SharedMaterialState &Materials::missingMaterial()
{
    return d->missingMaterial;
}

SharedMaterialState Materials::load(const QString &filename)
{
    QMutexLocker locker(&d->cleanupMutex);

    SharedMaterialState result;

    if (d->activeMaterials.contains(filename)) {
        result = d->activeMaterials[filename];
    }

    // It's possible mMaterials contains a null weakpointer, so we have to check again.
    if (!result) {
        result = SharedMaterialState(new MaterialState);

        if (!result->createFromFile(filename, d->renderStates, FileTextureSource::instance())) {
            qWarning("Unable to open material file %s: %s", qPrintable(filename), qPrintable(result->error()));
            result = missingMaterial();
        } else {
            d->activeMaterials[filename] = result;
        }
    }

    return result;
}

}

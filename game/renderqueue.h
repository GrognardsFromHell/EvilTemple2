#ifndef RENDERQUEUE_H
#define RENDERQUEUE_H

#include "renderstates.h"

namespace EvilTemple {

class Renderable;

/**
  A utility class that is used to assemble a list of objects for rendering.
  */
class RenderQueue
{
public:
    enum Category {
        Default = 0,
        ClippingGeometry,
        Lights,
        DebugOverlay,
        Count
    };

    void addRenderable(Category category, Renderable *renderable);

    const QList<Renderable*> queuedObjects(Category category) const;
    void clear();
private:
    QList<Renderable*> mQueuedObjects[Count];
};

inline void RenderQueue::addRenderable(Category category, Renderable *renderable)
{
    mQueuedObjects[category].append(renderable);
}

inline const QList<Renderable*> RenderQueue::queuedObjects(RenderQueue::Category category) const
{
    Q_ASSERT(category >= Default && category < Count);
    return mQueuedObjects[category];
}

inline void RenderQueue::clear()
{
    for (int i = Default; i < Count; ++i)
        mQueuedObjects[i].clear();
}

}

#endif // RENDERQUEUE_H

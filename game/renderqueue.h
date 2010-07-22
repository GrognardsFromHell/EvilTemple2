#ifndef RENDERQUEUE_H
#define RENDERQUEUE_H

#include "renderable.h"
#include "renderstates.h"

namespace EvilTemple {

class Renderable;

/**
  A utility class that is used to assemble a list of objects for rendering.
  */
class RenderQueue
{
public:
    void addRenderable(Renderable::Category category, Renderable *renderable);

    const QList<Renderable*> queuedObjects(Renderable::Category category) const;
    void clear();
private:
    QList<Renderable*> mQueuedObjects[Renderable::Count];
};

inline void RenderQueue::addRenderable(Renderable::Category category, Renderable *renderable)
{
    if (category < Renderable::Count) {
        mQueuedObjects[category].append(renderable);
    } else {
        qWarning("Invalid render category.");
    }
}

inline const QList<Renderable*> RenderQueue::queuedObjects(Renderable::Category category) const
{
    Q_ASSERT(category >= Renderable::Default && category < Renderable::Count);
    return mQueuedObjects[category];
}

inline void RenderQueue::clear()
{
    for (int i = Renderable::Default; i < Renderable::Count; ++i)
        mQueuedObjects[i].clear();
}

}

#endif // RENDERQUEUE_H
